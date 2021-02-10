byte currentSelectedItem = 0;
MenuItem MenuItems[] = {{0, true, "Set Brightness"}, {1, false, "Set Color"}, {2, false, "Exit"}};


MenuItem * getMenu() {
  return MenuItems;
}

void scrollMenuItemDown() {
  
}

void setupUI() {
  attachInterrupt(ROTARY_BUTTON_PIN, handleRotaryButtonInterrupt, FALLING);
  attachInterrupt(ROTARY_LEFT_PIN, handleRotarySpinInterrupt, CHANGE);
}

void handleRotarySpinInterrupt() {
  
}

void handleRotaryButtonInterrupt() {
  
}

void handleInput() {
  
}
