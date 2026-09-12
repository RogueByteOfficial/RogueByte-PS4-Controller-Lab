#ifndef ROGUEBYTE_REPORTS_H
#define ROGUEBYTE_REPORTS_H

#include "../controller/controller.h"
#include "../diagnostics/diagnostics.h"
#include <stdbool.h>

int report_generate_json(const DS4Controller* ctrl, const DiagnosticSession* diag, char* out_buf, size_t max_len);
int report_generate_text(const DS4Controller* ctrl, const DiagnosticSession* diag, char* out_buf, size_t max_len);
int report_save_to_disk(const char* dir_path, const char* json_data, const char* txt_data);

#endif /* ROGUEBYTE_REPORTS_H */
