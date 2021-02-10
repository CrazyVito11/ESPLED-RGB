class MenuItem {
  private:
    byte itemId;
    char displayName[16];
    bool isHighlighted;
  public:
    MenuItem(byte itemId, bool isHighlighted, char displayName[]) {
      this->itemId        = itemId;
      this->isHighlighted = isHighlighted;
      strcpy(this->displayName, displayName);
    }
    byte getItemId() {
      return this->itemId;
    }
    char * getDisplayName() {
      return this->displayName;
    }
}; 
