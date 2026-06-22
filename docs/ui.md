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
