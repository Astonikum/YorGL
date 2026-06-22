# UI

`org.yorgl.ui` is a small retained UI layer for clients that already have a renderer.

It owns:

- node measurement and layout;
- simple box, text, and button primitives;
- pointer dispatch;
- render commands through `YorCanvas`.

It does not own:

- game state;
- textures or fonts;
- input polling;
- Minecraft/FrostEngine screen rules.

## Public Types

- `YorRect` stores position and size and provides hit testing.
- `YorColor` stores linear RGBA values.
- `YorSize` stores measured size.
- `YorCanvas` is implemented by clients to draw rectangles and text.
- `YorNode` is the base UI contract.
- `YorBox` lays out children vertically.
- `YorText` emits text draw commands.
- `YorButton` draws a clickable button and calls an `onClick` lambda.

This layer is intentionally renderer-agnostic. A game can render `YorCanvas` through YorGL, another renderer, or tests.
