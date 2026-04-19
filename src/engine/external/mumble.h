 
 

#ifndef MUMBLE_H
#define MUMBLE_H

#include <wchar.h>
#include <stdbool.h>
#include <stdint.h>

#if !defined(MUMBLE_STUB) && (defined(__ANDROID__) || defined(__EMSCRIPTEN__))
	#define MUMBLE_STUB 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

 
struct MumbleLinkedMem {
	uint32_t uiVersion;  
	uint32_t uiTick;  
	float fAvatarPosition[3];  
	float fAvatarFront[3];  
	float fAvatarTop[3];  
	wchar_t name[256];  
	float fCameraPosition[3];  
	float fCameraFront[3];  
	float fCameraTop[3];  
	wchar_t identity[256];  
	uint32_t context_len;  
	unsigned char context[256];  
	wchar_t description[2048];  
};

 
struct MumbleContext;

 
struct MumbleContext* mumble_create_context(void);

 
struct MumbleContext* mumble_create_context_args(const char* name, const char* description);

 
void mumble_destroy_context(struct MumbleContext** context);

 
struct MumbleLinkedMem* mumble_get_linked_mem(struct MumbleContext* context);

 

 
bool mumble_relink_needed(struct MumbleContext* context);

 
bool mumble_set_name(struct MumbleContext* context, const char* name);

 
bool mumble_set_identity(struct MumbleContext* context, const char* identity);

 
bool mumble_set_context(struct MumbleContext* context, const char* mumbleContext);

 
bool mumble_set_description(struct MumbleContext* context, const char* description);

 

 
void mumble_2d_update(struct MumbleContext* context, float x, float y);

 
void mumble_3d_update(struct MumbleContext* context, float x, float y, float z);

#ifdef __cplusplus
}
#endif

#endif
