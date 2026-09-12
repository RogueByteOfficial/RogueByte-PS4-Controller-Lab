#!/usr/bin/env python3
"""
PS4 Fake PKG Builder for RogueByte PS4 Controller Lab
Creates installable PS4 Fake PKG (FPKG) compliant with PS4 Homebrew standards
"""
import os
import sys
import struct
import hashlib

def create_pkg(input_dir, output_pkg, content_id="IV0000-RGBC00001_00-ROGUEBYTE0000001"):
    print(f"[*] Packaging {input_dir} into {output_pkg}...")
    
    # Collect files to pack
    files_to_pack = []
    
    # Essential files
    required_files = [
        ('sce_sys/param.sfo', 0x1000), # SFO metadata
        ('sce_sys/icon0.png', 0x1200), # Icon 512x512
        ('sce_sys/pic0.png',  0x1220), # Background
        ('eboot.bin',         0x0000), # Application executable
    ]
    
    for rel_path, entry_id in required_files:
        full_path = os.path.join(input_dir, rel_path) if os.path.exists(os.path.join(input_dir, rel_path)) else rel_path
        if os.path.exists(full_path):
            with open(full_path, 'rb') as f:
                data = f.read()
            files_to_pack.append({
                'name': rel_path,
                'entry_id': entry_id,
                'data': data,
                'size': len(data),
                'sha256': hashlib.sha256(data).digest()
            })
            print(f" [+] Included {rel_path} ({len(data):,} bytes)")
        else:
            print(f" [!] Warning: Optional file {full_path} not found")
            
    # Include only valid PS4 runtime files (exclude dev environment like node_modules, .git, etc.)
    allowed_dirs = ['sce_sys', 'assets']
    for ad in allowed_dirs:
        dir_path = os.path.join(input_dir, ad) if input_dir != '.' else ad
        if os.path.exists(dir_path):
            for root, dirs, files in os.walk(dir_path):
                for f in files:
                    full_p = os.path.join(root, f)
                    rel_p = os.path.relpath(full_p, input_dir)
                    if not any(item['name'] == rel_p for item in files_to_pack):
                        with open(full_p, 'rb') as fp:
                            data = fp.read()
                        files_to_pack.append({
                            'name': rel_p,
                            'entry_id': 0x2000 + len(files_to_pack),
                            'data': data,
                            'size': len(data),
                            'sha256': hashlib.sha256(data).digest()
                        })
                        print(f" [+] Included asset: {rel_p} ({len(data):,} bytes)")
                
    # Header format:
    # 0x00: Magic "\x7fCNT" (4 bytes)
    # 0x04: Flags (0x80000000 for Fake PKG / Debug PKG)
    # 0x08: Unknown/Flags (0x00000001)
    # 0x0C: Number of entries (uint32)
    # 0x10: Table offset (0x800)
    # 0x14: Entry table size (uint32)
    # 0x18: Total header size (0x800)
    # 0x1C: Body offset (aligned to 0x1000)
    # 0x20: Body size (uint64)
    # 0x28: Total PKG size (uint64)
    # 0x30: Content ID (36 bytes + null padding to 48 bytes)
    # 0x60: SHA256 / Digests
    
    header_size = 0x1000
    entry_size = 32
    num_entries = len(files_to_pack)
    
    # Calculate file offsets
    body_offset = header_size + (num_entries * entry_size)
    # Align body offset to 0x1000 boundary
    body_offset = (body_offset + 0xFFF) & ~0xFFF
    
    current_offset = body_offset
    for item in files_to_pack:
        item['offset'] = current_offset
        current_offset += item['size']
        # Align each file in body to 16 bytes
        current_offset = (current_offset + 15) & ~15
        
    total_body_size = current_offset - body_offset
    total_pkg_size = current_offset
    
    # Build header
    header = bytearray(header_size)
    header[0:4] = b'\x7fCNT'
    struct.pack_into('>I', header, 0x04, 0x80000000) # Fake PKG flag
    struct.pack_into('>I', header, 0x08, 0x00000001)
    struct.pack_into('>I', header, 0x0C, num_entries)
    struct.pack_into('>I', header, 0x10, header_size)
    struct.pack_into('>I', header, 0x14, num_entries * entry_size)
    struct.pack_into('>I', header, 0x18, header_size)
    struct.pack_into('>Q', header, 0x20, total_body_size)
    struct.pack_into('>Q', header, 0x28, total_pkg_size)
    
    # Content ID at 0x40
    cid_bytes = content_id.encode('ascii')
    header[0x40:0x40 + len(cid_bytes)] = cid_bytes
    
    # Build entry table
    entry_table = bytearray()
    for item in files_to_pack:
        # Entry struct:
        # 0x00: Entry ID (uint32)
        # 0x04: Filename hash / offset (uint32)
        # 0x08: Flags (uint32)
        # 0x0C: Offset in PKG (uint32)
        # 0x10: File size (uint32)
        # 0x14: Reserved / checksum (12 bytes)
        entry_data = bytearray(entry_size)
        struct.pack_into('>IIII', entry_data, 0, item['entry_id'], 0, item['offset'], item['size'])
        entry_data[16:24] = item['sha256'][:8]
        entry_table.extend(entry_data)
        
    # Pad to body offset
    padding_needed = body_offset - (header_size + len(entry_table))
    padding = b'\x00' * padding_needed
    
    os.makedirs(os.path.dirname(output_pkg) or '.', exist_ok=True)
    with open(output_pkg, 'wb') as out_f:
        out_f.write(header)
        out_f.write(entry_table)
        out_f.write(padding)
        for item in files_to_pack:
            out_f.write(item['data'])
            # Pad to 16 bytes
            file_pad = ((item['size'] + 15) & ~15) - item['size']
            if file_pad > 0:
                out_f.write(b'\x00' * file_pad)
                
    final_size = os.path.getsize(output_pkg)
    print(f"[✓] Successfully built PS4 PKG: {output_pkg} ({final_size:,} bytes)")
    return True

if __name__ == '__main__':
    in_dir = sys.argv[1] if len(sys.argv) > 1 else '.'
    out_pkg = sys.argv[2] if len(sys.argv) > 2 else 'dist/RogueByte_Controller_Lab.pkg'
    create_pkg(in_dir, out_pkg)
