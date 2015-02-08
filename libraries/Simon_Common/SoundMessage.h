#ifndef SoundMessage_h
#define SoundMesage_h

// Data Transfer; Sound Types
#define TYPE_STOP 0
#define TYPE_BAFF 1;
#define TYPE_WIN 2;
#define TYPE_LOSE 3;
#define TYPE_ROCK 4;

#define TYPE_DEFAULT TYPE_STOP;
#define PLAY_COUNT_DEFAULT 1;

// Used for instructing the music module on what to play.
struct SoundMessage {
    int type;
    int playCount;
    int volume;
};
#endif

