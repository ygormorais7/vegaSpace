#ifndef AUDIO_H
#define AUDIO_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>

// Funções de controle de áudio
void audio_init();
void audio_cleanup();

void audio_load_music(const std::string& filepath);
void audio_load_sfx(const std::string& filepath);

void audio_play_music_loop();
void audio_stop_music();
void audio_play_sfx();

#endif // AUDIO_H