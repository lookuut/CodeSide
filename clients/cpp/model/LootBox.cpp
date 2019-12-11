#include "LootBox.hpp"

LootBox::LootBox() { }
LootBox::LootBox(Vec2Double position, Vec2Double size, std::shared_ptr<Item> item) : position(position), size(size), item(item) {
    leftTop = Vec2Double(position.x - size.x / 2.0, position.y + size.y );
    rightDown = Vec2Double(position.x + size.x / 2.0, position.y);
}
LootBox LootBox::readFrom(InputStream& stream) {
    LootBox result;
    result.position = Vec2Double::readFrom(stream);
    result.size = Vec2Double::readFrom(stream);
    result.item = Item::readFrom(stream);

    result.leftTop = Vec2Double(result.position.x - result.size.x / 2.0, result.position.y + result.size.y );
    result.rightDown = Vec2Double(result.position.x + result.size.x / 2.0, result.position.y);

    return result;
}
void LootBox::writeTo(OutputStream& stream) const {
    position.writeTo(stream);
    size.writeTo(stream);
    item->writeTo(stream);
}
std::string LootBox::toString() const {
    return std::string("LootBox") + "(" +
        position.toString() +
        size.toString() +
        item->toString() +
        ")";
}
