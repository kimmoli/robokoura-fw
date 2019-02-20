#ifndef TK_SHELLCOMMANDS_H
#define TK_SHELLCOMMANDS_H

#include "hal.h"
#include "shell.h"

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)

extern void cmd_reboot(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_diic(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_piic(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_deep(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_peep(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_dmw(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_dmb(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_probe(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_ps(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_stepper(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_neopixel(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_servo(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_accel(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_oled(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_volts(BaseSequentialStream *chp, int argc, char *argv[]);
extern void cmd_pid(BaseSequentialStream *chp, int argc, char *argv[]);

extern const ShellCommand commands[];
extern const ShellConfig shell_cfg_uart;

#endif // TK_SHELLCOMMANDS_H
