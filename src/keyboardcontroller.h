#ifndef KEYBOARDCONTROLLER_H
#define KEYBOARDCONTROLLER_H

#include <stack>

class KeyboardController {
public:
    KeyboardController();

    void keyPressEvent(char keycode) { currentKeybuffer_[keycode] = true;}
    void keyReleaseEvent(char keycode) { currentKeybuffer_[keycode] = false;}
    void swapBuffers();

    bool isKeyDown(char keycode) const { return currentKeybuffer_[keycode]; }
    bool isKeyPress(char keycode) const { return currentKeybuffer_[keycode] && !prevKeybuffer_[keycode]; }
    bool isKeyUp(char keycode) const { return !currentKeybuffer_[keycode] && prevKeybuffer_[keycode]; }

protected:

    bool *currentKeybuffer_;
    bool *prevKeybuffer_;

    bool keyboardBuffer0_[256];
    bool keyboardBuffer1_[256];

};

#endif // KEYBOARDCONTROLLER_H
