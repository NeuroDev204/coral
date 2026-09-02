#include "codedefs.h"
#include "slout.h"
#include "pt_defs.h"
#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <locale.h>
#include <wchar.h>
#include <math.h>
#include <map>
#include <mutex>
#include <string>

static std::mutex g_reg_mutex;
static std::map<std::wstring, std::wstring> g_virtual_registry;

static void WcharToChar(const wchar_t* wstr, char* cstr, size_t max_len) {
    if (!wstr || !cstr) return;
    wcstombs(cstr, wstr, max_len);
}

static void CharToWchar(const char* cstr, wchar_t* wstr, size_t max_len) {
    if (!wstr || !cstr) return;
    mbstowcs(wstr, cstr, max_len);
}

FILE* fileOpen_Wide(wchar_t *wcp_name, wchar_t *wcp_mode, CSlout *hp_slout) {
    char name[1024] = {0};
    char mode[16] = {0};
    WcharToChar(wcp_name, name, sizeof(name));
    WcharToChar(wcp_mode, mode, sizeof(mode));
    return fopen(name, mode);
}

int fileExist_Wide(wchar_t *wcp_name, int *ip_exist) {
    char name[1024] = {0};
    WcharToChar(wcp_name, name, sizeof(name));
    struct stat buffer;
    *ip_exist = (stat(name, &buffer) == 0);
    return OKAY;
}

// Read preset name from .fac file - simplified parser for Linux
// The preset name is on line 3 (after "CLASS1 : Effect Type" and "9: Version")
int readPresetName_Wide(wchar_t *wcp_filename, wchar_t *wcp_preset_name, int i_max_len) {
    char name[1024] = {0};
    WcharToChar(wcp_filename, name, sizeof(name));

    FILE *f = fopen(name, "r");
    if (!f) return NOT_OKAY;

    char line[256];
    int line_num = 0;

    while (fgets(line, sizeof(line), f) && line_num < 3) {
        line_num++;
        if (line_num == 3) {
            // Remove trailing newline
            size_t len = strlen(line);
            if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
            if (len > 1 && line[len-2] == '\r') line[len-2] = '\0';

            // Convert to wide char
            CharToWchar(line, wcp_preset_name, i_max_len);
            fclose(f);
            return OKAY;
        }
    }

    fclose(f);
    return NOT_OKAY;
}

int pstrCovertUTF8StringToWideCharString_WithAlloc(char *cp_string, wchar_t **wcpp_string, int *ip_strlen) {
    if (!cp_string || !wcpp_string || !ip_strlen) return NOT_OKAY;

    // Use mbstowcs with proper size calculation for UTF-8
    // First, get the required size
    setlocale(LC_ALL, "");  // Set locale for proper UTF-8 handling

    // Count actual characters needed
    size_t wide_len = mbstowcs(nullptr, cp_string, 0);
    if (wide_len == (size_t)-1) {
        // Conversion failed, try simple fallback
        size_t len = strlen(cp_string);
        *wcpp_string = (wchar_t*)calloc(len + 1, sizeof(wchar_t));
        if (!*wcpp_string) return NOT_OKAY;
        *ip_strlen = (int)len;
        CharToWchar(cp_string, *wcpp_string, len + 1);
        return OKAY;
    }

    *wcpp_string = (wchar_t*)calloc(wide_len + 1, sizeof(wchar_t));
    if (!*wcpp_string) return NOT_OKAY;

    size_t result = mbstowcs(*wcpp_string, cp_string, wide_len + 1);
    if (result == (size_t)-1) {
        free(*wcpp_string);
        *wcpp_string = nullptr;
        return NOT_OKAY;
    }

    *ip_strlen = (int)result;
    return OKAY;
}

int pstrCovertWideCharStringToUTF8String_WithAlloc(wchar_t *wcp_string, char **cpp_string, int *ip_strlen) {
    if (!wcp_string || !cpp_string || !ip_strlen) return NOT_OKAY;
    size_t len = wcslen(wcp_string);
    size_t max_bytes = (len + 1) * 4;
    *cpp_string = (char*)calloc(max_bytes, sizeof(char));
    *ip_strlen = (int)max_bytes;
    WcharToChar(wcp_string, *cpp_string, max_bytes);
    return OKAY;
}

int regReadKey_Wide(int i_key_class, wchar_t *wcp_keyname, int *ip_key_exists_flag, wchar_t *wcp_value, unsigned long ul_value_size) {
    if (!wcp_keyname) {
        if (ip_key_exists_flag) *ip_key_exists_flag = 0;
        return NOT_OKAY;
    }
    std::lock_guard<std::mutex> lock(g_reg_mutex);
    auto it = g_virtual_registry.find(wcp_keyname);
    if (it != g_virtual_registry.end()) {
        if (ip_key_exists_flag) *ip_key_exists_flag = 1;
        if (wcp_value && ul_value_size > 0) {
            wcsncpy(wcp_value, it->second.c_str(), ul_value_size - 1);
            wcp_value[ul_value_size - 1] = L'\0';
        }
    } else {
        if (ip_key_exists_flag) *ip_key_exists_flag = 0;
        if (wcp_value && ul_value_size > 0) {
            wcp_value[0] = L'\0';
        }
    }
    return OKAY;
}

int regRemoveKey_Wide(int i_key_class, wchar_t *wcp_keyname) {
    if (!wcp_keyname) return NOT_OKAY;
    std::lock_guard<std::mutex> lock(g_reg_mutex);
    g_virtual_registry.erase(wcp_keyname);
    return OKAY;
}

int regKeyExist_Wide(int i_key_class, wchar_t *wcp_keyname, int *ip_key_exists_flag) {
    if (!wcp_keyname) {
        if (ip_key_exists_flag) *ip_key_exists_flag = 0;
        return NOT_OKAY;
    }
    std::lock_guard<std::mutex> lock(g_reg_mutex);
    if (ip_key_exists_flag) {
        *ip_key_exists_flag = (g_virtual_registry.find(wcp_keyname) != g_virtual_registry.end()) ? 1 : 0;
    }
    return OKAY;
}

int pstrCalcLocationOfStrInStr_Wide(wchar_t* p1, wchar_t* p2, int flag, int* out1, int* out2) {
    return OKAY;
}

void mthCalcQuantDelta(float r_range_min, float r_range_max, int i_number_of_levels, float *rp_delta) {
    float rough_delta = (r_range_max - r_range_min)/(float)(i_number_of_levels - 1);
    float log_rough_delta = (float)log10((double)rough_delta);
    int i_ten_power = (int)log_rough_delta;
    float remainder = log_rough_delta - i_ten_power;
    float delta_val;
    
    if( log_rough_delta < 0.0) {
        i_ten_power = (int)log_rough_delta - 1;
        remainder += 1.0f;
    }
    if( remainder == 0.0f ) {
        delta_val = rough_delta;
    } else {
        delta_val = 10.0f; 
        if( (float)log10(5.0) > remainder ) delta_val = 5.0f;
        if( (float)log10(2.5) > remainder ) delta_val = 2.5f;
        if( (float)log10(2.0) > remainder ) delta_val = 2.0f;
        if( i_ten_power > 0 )
            for(int index=0; index<i_ten_power; index++) delta_val *= 10.0f;
        if( i_ten_power < 0 )
            for(int index=0; index<(-i_ten_power); index++) delta_val *= 0.1f;
    }
    *rp_delta = delta_val;
}

float mthCalcRoundedValue(float r_input_value, float r_delta, float r_output_min, float r_output_max) {                      
    long l_tmp;
    float r_tmp = r_input_value / r_delta;
    if( r_tmp >= 0.0f ) r_tmp += 0.5f;
    else r_tmp -= 0.5f;
    l_tmp = (long)r_tmp;
    r_tmp = (float)(l_tmp * r_delta);
    if( r_tmp < r_output_min ) r_tmp = r_output_min;
    if( r_tmp > r_output_max ) r_tmp = r_output_max;
    return r_tmp;
}

float mthCalcClosestNiceValue(float r_input_value, float r_num_places) {
    int num_decimal_places = (int)log10(r_input_value) + 1;
    float remainder = r_num_places - (int)r_num_places;
    float log_round_val = num_decimal_places - (int)r_num_places + (float)log10(remainder);
    float round_val = (float)pow(10.0, (double)log_round_val);
    remainder = (float)fmod(r_input_value, round_val);
    int i_tmp = (int)(r_input_value/round_val);
    float r_tmp = (float)(i_tmp * round_val);
    if( remainder >= (round_val * 0.5f) ) r_tmp += round_val;
    return r_tmp;
}

int mthMidiOctaveFreqsPara(float* fp1, int i1) {
    return OKAY;
}



int regCreateKey_Wide(int i_key_class, wchar_t *wcp_keyname, wchar_t *wcp_value) {
    if (!wcp_keyname) return NOT_OKAY;
    std::lock_guard<std::mutex> lock(g_reg_mutex);
    g_virtual_registry[wcp_keyname] = wcp_value ? wcp_value : L"";
    return OKAY;
}

int mthIsLong_Wide(wchar_t *wcp_string, int *ip_is_long) {
    *ip_is_long = IS_FALSE; 
    if (wcp_string == NULL) return NOT_OKAY;
    int length = (int)wcslen(wcp_string);
    if (length <= 0) return OKAY;
    int done = IS_FALSE;
    int index = 0;   
    while (!done) {
        if ((wcp_string[index] < L'0') || (wcp_string[index] > L'9')) {
            if (index != 0) done = IS_TRUE;
            else if (wcp_string[index] != '-') done = IS_TRUE;
        }
        if (!done) {
            index++;
            if (index == length) {
                done = IS_TRUE;
                *ip_is_long = IS_TRUE;
            }
        }
    }
    return OKAY;
}

// Linux stubs for hardware EEPROM functions (not used on Linux, but required by DSP library)
extern "C" int comHrdEepromReadUlong(short unsigned board_address, short unsigned address, unsigned long *ulpdata) {
    // On Linux, we don't have hardware EEPROM access
    // Return OKAY with a default value to allow preset loading to proceed
    if (ulpdata) *ulpdata = 0;
    return OKAY;
}

extern "C" int comHrdEepromWriteUlong(short unsigned board_address, short unsigned address, unsigned long uldata) {
    // On Linux, we don't have hardware EEPROM access
    // Just return OKAY to indicate success (no-op)
    return OKAY;
}

// Stub implementations for other required DSP functions
int hutBootLoad(char *file_name, unsigned short us_processor_num) {
    return OKAY;  // No-op on Linux
}

int hutsyncCheckOutDsp(unsigned short us_proc_addr) {
    return OKAY;  // No-op on Linux
}

void hutsyncCheckInDsp(unsigned short us_proc_addr) {
    // No-op on Linux
}

void hutsyncResetDsp(unsigned short us_proc_addr) {
    // No-op on Linux
}

int hutmeterCheckIfNew(unsigned short us_proc_address) {
    return OKAY;  // No-op on Linux
}

void hutmeterGetMeterValues(unsigned short us_proc_address, struct hardwareMeterValType *meter_data) {
    // No-op on Linux, zero out the meter data
    if (meter_data) {
        meter_data->values_are_new = 0;
        meter_data->left_in = 0;
        meter_data->right_in = 0;
        meter_data->left_out = 0;
        meter_data->right_out = 0;
        meter_data->dsp_status = 0;
        for (int i = 0; i < 8; i++) meter_data->aux_vals[i] = 0.0f;
    }
}

int hutmeterCheckIfNewSc(unsigned short us_proc_address) {
    return OKAY;  // No-op on Linux
}

void hutmeterGetMeterValuesSc(unsigned short us_proc_address, struct hardwareMeterValType *meter_data) {
    // No-op on Linux
    if (meter_data) {
        meter_data->values_are_new = 0;
        meter_data->left_in = 0;
        meter_data->right_in = 0;
        meter_data->left_out = 0;
        meter_data->right_out = 0;
        meter_data->dsp_status = 0;
        for (int i = 0; i < 8; i++) meter_data->aux_vals[i] = 0.0f;
    }
}

// Stub for comHrdwrGetChar - reads a character from hardware
extern "C" int comHrdwrGetChar(int board_address, long *lp_val) {
    if (lp_val) *lp_val = 0;
    return OKAY;
}
