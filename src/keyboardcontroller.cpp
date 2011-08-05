#include "keyboardcontroller.h"
#include <string.h>
#include <assert.h>
#include "common.h"
KeyboardController::KeyboardController() {
    currentKeybuffer_ = keyboardBuffer0_;
    prevKeybuffer_ = keyboardBuffer1_;

    memset(currentKeybuffer_, 0, sizeof(bool)*256);
    memset(prevKeybuffer_, 0, sizeof(bool)*256);

}

void KeyboardController::swapBuffers() {
    bool *tmp = prevKeybuffer_;
    prevKeybuffer_ = currentKeybuffer_;
    currentKeybuffer_ = tmp;
    memcpy(currentKeybuffer_, prevKeybuffer_, sizeof(bool)*256);
}
