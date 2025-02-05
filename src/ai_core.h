#ifndef AI_CORE_H
#define AI_CORE_H

// AI Boot Optimization
void ai_analyze_boot_time(int boot_time_ms);
int ai_should_enable_fastboot(void);
void ai_optimize_boot_sequence(void);

// AI Hardware Optimization
void ai_monitor_health(int cpu_temp, int disk_status);
void ai_optimize_hardware(void);

// AI BIOS Menu Enhancements
int ai_recommend_setting(const char *setting);
void ai_customize_bios_interface(void);
const char* ai_generate_menu_description(const char *text);
const char* ai_generate_help_text(const char *help);

#endif // AI_CORE_H

