#pragma once

#ifdef AUDIO_OAL
#include "eax.h"

#ifdef AUDIO_OAL_NO_EFX
#include "efx_stub.h"
#else
#include "AL/efx.h"
#endif

void EFXInit();
void EAX3_Set(ALuint effect, const EAXLISTENERPROPERTIES *props);
void EFX_Set(ALuint effect, const EAXLISTENERPROPERTIES *props);
void EAX3_SetReverbMix(ALuint filter, float mix);
void SetEffectsLevel(ALuint uiFilter, float level);

namespace re3_openal {

#ifdef AUDIO_OAL_NO_EFX
inline void alGenEffects(ALsizei n, ALuint *e) { for (ALsizei i = 0; i < n; i++) e[i] = 0; }
inline void alDeleteEffects(ALsizei, const ALuint *) {}
inline ALboolean alIsEffect(ALuint) { return AL_FALSE; }
inline void alEffecti(ALuint, ALenum, ALint) {}
inline void alGenAuxiliaryEffectSlots(ALsizei n, ALuint *s) { for (ALsizei i = 0; i < n; i++) s[i] = 0; }
inline void alDeleteAuxiliaryEffectSlots(ALsizei, const ALuint *) {}
inline ALboolean alIsAuxiliaryEffectSlot(ALuint) { return AL_FALSE; }
inline void alAuxiliaryEffectSloti(ALuint, ALenum, ALint) {}
inline void alGenFilters(ALsizei n, ALuint *f) { for (ALsizei i = 0; i < n; i++) f[i] = 0; }
inline void alDeleteFilters(ALsizei, const ALuint *) {}
inline ALboolean alIsFilter(ALuint) { return AL_FALSE; }
inline void alFilteri(ALuint, ALenum, ALint) {}
inline void alFilterf(ALuint, ALenum, ALfloat) {}
#else
extern LPALGENEFFECTS alGenEffects;
extern LPALDELETEEFFECTS alDeleteEffects;
extern LPALISEFFECT alIsEffect;
extern LPALEFFECTI alEffecti;
extern LPALEFFECTIV alEffectiv;
extern LPALEFFECTF alEffectf;
extern LPALEFFECTFV alEffectfv;
extern LPALGETEFFECTI alGetEffecti;
extern LPALGETEFFECTIV alGetEffectiv;
extern LPALGETEFFECTF alGetEffectf;
extern LPALGETEFFECTFV alGetEffectfv;
extern LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
extern LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
extern LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
extern LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
extern LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv;
extern LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
extern LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv;
extern LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti;
extern LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv;
extern LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf;
extern LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv;
extern LPALGENFILTERS alGenFilters;
extern LPALDELETEFILTERS alDeleteFilters;
extern LPALISFILTER alIsFilter;
extern LPALFILTERI alFilteri;
extern LPALFILTERIV alFilteriv;
extern LPALFILTERF alFilterf;
extern LPALFILTERFV alFilterfv;
extern LPALGETFILTERI alGetFilteri;
extern LPALGETFILTERIV alGetFilteriv;
extern LPALGETFILTERF alGetFilterf;
extern LPALGETFILTERFV alGetFilterfv;
#endif

}

using namespace re3_openal;

#endif
