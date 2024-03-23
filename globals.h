#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#ifdef __cplusplus
extern "C" {
#endif

// Preview Project
bool can_previewProject = false;

// Entities List Panel
extern Rectangle panelRec;

// Entities List Box
extern Rectangle panelContentRec;
extern Vector2 panelScroll;

extern bool showContentArea;

#define screenWidth    GetScreenWidth()
#define screenHeight   GetScreenHeight()

std::thread::id mainThreadId;
bool bloomEnabled = false;
float bloomBrightness = 0.0f;
float bloomSamples = 3.0f;

#ifdef __cplusplus
}
#endif

#endif // GLOBALS_H
