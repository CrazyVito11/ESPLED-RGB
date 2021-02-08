// todo: implement class functions

class MenuItem {
  private:
    byte itemId;
  public:
    MenuItem(byte itemId) {
      this->itemId = itemId;
    }
    byte getItemId() {
      return this->itemId;
    }
};
