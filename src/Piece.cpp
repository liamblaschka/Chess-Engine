#include "Piece.h"

Colour oppositeColour(Colour colour) {
    if (colour == Colour::White) {
        return Colour::Black;
    } else if (colour == Colour::Black) {
        return Colour::White;
    }
    return Colour::None;
}
