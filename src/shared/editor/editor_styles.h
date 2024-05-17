#ifndef __EDITOR_STYLES_H__
#define __EDITOR_STYLES_H__

#include "editor.h"
#include "shared/ui/dusk-gui.h"
#include "shared/util/arena.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern DuskGuiStyleGroup _editor_titleStyleGroup;
extern DuskGuiStyleGroup _editor_closeButtonStyleGroup;
extern DuskGuiStyleGroup _editor_resizeAreaStyleGroup;
extern DuskGuiStyleGroup _editor_invisibleStyleGroup;
extern DuskGuiStyleGroup _editor_labelCenteredStyleGroup;

extern Texture2D _editor_iconHierarchy;
extern Texture2D _editor_iconInspector;
extern Texture2D _editor_iconPlay;
extern Texture2D _editor_iconPause;
extern Texture2D _editor_iconStep;

void EditorStyles_init();

#endif