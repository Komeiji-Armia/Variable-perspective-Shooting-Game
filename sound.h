#pragma comment(lib, "winmm.lib")
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

#define SOUND_HIT_PLAYER "sound/hit_player.wav"
#define SOUND_SPECIAL    "sound/special.wav"
#define SOUND_SHOOT      "sound/shoot_cut.wav"
#define SOUND_EXTEND     "sound/extend.wav"

void soundInit(void)
{
    char cmd[256];
    sprintf(cmd, "open %s type waveaudio alias HIT_PLAYER", SOUND_HIT_PLAYER);
    mciSendString(cmd, NULL, 0, NULL);
    sprintf(cmd, "open %s type waveaudio alias SPECIAL",    SOUND_SPECIAL);
    mciSendString(cmd, NULL, 0, NULL);
    sprintf(cmd, "open %s type waveaudio alias SHOOT",      SOUND_SHOOT);
    mciSendString(cmd, NULL, 0, NULL);
    sprintf(cmd, "open %s type waveaudio alias EXTEND",     SOUND_EXTEND);
    mciSendString(cmd, NULL, 0, NULL);
}

void soundHitPlayer(void)
{
    PlaySound(SOUND_HIT_PLAYER, NULL, SND_FILENAME | SND_ASYNC | SND_NOSTOP);
}

void soundSpecial(void)
{
    PlaySound(SOUND_SPECIAL, NULL, SND_FILENAME | SND_ASYNC | SND_NOSTOP);
}

void soundExtend(void)
{
    PlaySound(SOUND_EXTEND, NULL, SND_FILENAME | SND_ASYNC | SND_NOSTOP);
}

void soundShoot(void)
{
    PlaySound(SOUND_SHOOT, NULL, SND_FILENAME | SND_ASYNC | SND_NOSTOP);
}

void soundClose(void)
{
    mciSendString("close HIT_PLAYER", NULL, 0, NULL);
    mciSendString("close SPECIAL",    NULL, 0, NULL);
    mciSendString("close SHOOT",      NULL, 0, NULL);
}