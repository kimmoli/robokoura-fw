#include "hal.h"
#include "shell.h"
#include "shellcommands.h"

char uartShellHistoryBuffer[SHELL_MAX_HIST_BUFF];

const ShellCommand commands[] =
{
    { "reboot",  cmd_reboot },
    { "dmb",     cmd_dmb },
    { "dmw",     cmd_dmw },
    { "diic",    cmd_diic },
    { "piic",    cmd_piic },
    { "deep",    cmd_deep },
    { "peep",    cmd_peep },
    { "probe",   cmd_probe },
    { "ps",      cmd_ps },
    { "step",    cmd_stepper },
    { "neo",     cmd_neopixel },
    { "servo",   cmd_servo },
    { "acc",     cmd_accel },
    { "oled",    cmd_oled },
    { "volts",    cmd_volts },
    {NULL, NULL}
};

const ShellConfig shell_cfg_uart =
{
    (BaseSequentialStream *)&SD3,
    commands,
    uartShellHistoryBuffer,
    SHELL_MAX_HIST_BUFF
};
