#!/usr/bin/env python3
"""
PS4 param.sfo Generator for RogueByte PS4 Controller Lab
Generates valid Sony PlayStation 4 SFO metadata file
"""
import struct
import os

def create_sfo(entries, output_path):
    # Sort entries by key name alphabetically (mandatory for SFO spec)
    sorted_entries = sorted(entries.items(), key=lambda x: x[0])
    
    key_table = bytearray()
    data_table = bytearray()
    index_table = bytearray()
    
    key_offsets = []
    for key, (fmt, max_len, val) in sorted_entries:
        key_offsets.append(len(key_table))
        key_table.extend(key.encode('utf-8') + b'\x00')
        
    for i, (key, (fmt, max_len, val)) in enumerate(sorted_entries):
        data_offset = len(data_table)
        if fmt == 0x0404: # Integer
            encoded_val = struct.pack('<I', val)
            data_len = 4
        else: # UTF-8 string
            encoded_val = val.encode('utf-8') + b'\x00'
            data_len = len(encoded_val)
            # Pad to max_len if needed
            if data_len < max_len:
                encoded_val += b'\x00' * (max_len - data_len)
                
        data_table.extend(encoded_val)
        # Pad data_table to 4-byte alignment
        while len(data_table) % 4 != 0:
            data_table.append(0)
            
        index_table.extend(struct.pack('<HHIII', key_offsets[i], fmt, data_len, max_len, data_offset))
        
    # SFO Header
    magic = b'\x00PSF'
    version = 0x00000101
    key_table_start = 20 + len(index_table)
    data_table_start = key_table_start + len(key_table)
    # Pad key_table to 4-byte boundary
    key_padding = (4 - (len(key_table) % 4)) % 4
    data_table_start += key_padding
    
    header = struct.pack('<4sIIII', magic, version, key_table_start, data_table_start, len(sorted_entries))
    
    with open(output_path, 'wb') as f:
        f.write(header)
        f.write(index_table)
        f.write(key_table)
        f.write(b'\x00' * key_padding)
        f.write(data_table)
        
    print(f"Generated {output_path} ({os.path.getsize(output_path)} bytes)")

if __name__ == '__main__':
    sfo_data = {
        'APP_TYPE': (0x0404, 4, 1),
        'CATEGORY': (0x0204, 4, 'gd'),
        'CONTENT_ID': (0x0204, 48, 'IV0000-RGBC00001_00-ROGUEBYTE0000001'),
        'DOWNLOAD_DATA_SIZE': (0x0404, 4, 0),
        'PARENTAL_LEVEL': (0x0404, 4, 1),
        'SYSTEM_VER': (0x0404, 4, 0x05050000),
        'TITLE': (0x0204, 128, 'RogueByte Controller Lab'),
        'TITLE_ID': (0x0204, 12, 'RGBC00001'),
        'VERSION': (0x0204, 8, '01.00'),
    }
    os.makedirs('sce_sys', exist_ok=True)
    create_sfo(sfo_data, 'sce_sys/param.sfo')
