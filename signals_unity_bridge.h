//
// Copyright (C) 2019 Motion Spell
//

#pragma once
#ifndef SUB_H
#define SUB_H

#include <cstdint>
#include <cstddef> // size_t

#ifdef _WIN32
#ifdef BUILDING_DLL
#define SUB_EXPORT __declspec(dllexport)
#else
#define SUB_EXPORT __declspec(dllimport)
#endif
#else
#define SUB_EXPORT __attribute__((visibility("default")))
#endif

struct FrameInfo
{
  // presentation timestamp, in milliseconds units.
  int64_t timestamp;
};

extern "C" {
// opaque handle to a signals pipeline
struct sub_handle;

// Creates a new pipeline.
// name: a display name for log messages. Can be NULL.
// The returned pipeline must be freed using 'sub_destroy'.
SUB_EXPORT sub_handle* sub_create(const char* name);

// Destroys a pipeline. This frees all the resources.
SUB_EXPORT void sub_destroy(sub_handle* h);

// Returns the number of compressed streams.
SUB_EXPORT int sub_get_stream_count(sub_handle* h);

// Plays a given URL.
SUB_EXPORT bool sub_play(sub_handle* h, const char* URL);

// Copy the next received compressed frame to a buffer.
// Returns: the size of compressed data actually copied,
// or zero, if no frame was available for this stream.
// If 'dst' is null, the frame will not be dequeued, but its size will be returned.
SUB_EXPORT size_t sub_grab_frame(sub_handle* h, int streamIndex, uint8_t* dst, size_t dstLen, FrameInfo* info);

//auto func_sub_create = IMPORT(sub_create);
//auto func_sub_play = IMPORT(sub_play);
//auto func_sub_destroy = IMPORT(sub_destroy);
//auto func_sub_grab_frame = IMPORT(sub_grab_frame);

/*
typedef ULONG(FAR PASCAL APIfunc_sub_create)(const char* FAR name);
typedef APIfunc_sub_create FAR *LPAPIfunc_sub_create;
#undef APIfunc_sub_create 
#define APIfunc_sub_create (*func_sub_create) 
LPAPIfunc_sub_create func_sub_create;

typedef ULONG(FAR PASCAL APIDquitaUnArticulo)(char FAR *name);
typedef APIDquitaUnArticulo FAR *LPAPIDquitaUnArticulo;
#undef APIDquitaUnArticulo 
#define APIDquitaUnArticulo (*lpfnAPIDquitaUnArticulo) 
LPAPIDquitaUnArticulo lpfnAPIDquitaUnArticulo;

typedef ULONG(FAR PASCAL APIDencuentraUnArticulo)(char FAR *name, BUFC FAR *vbas);
typedef APIDencuentraUnArticulo FAR *LPAPIDencuentraUnArticulo;
#undef APIDencuentraUnArticulo 
#define APIDencuentraUnArticulo (*lpfnAPIDencuentraUnArticulo) 
LPAPIDencuentraUnArticulo lpfnAPIDencuentraUnArticulo;

typedef ULONG(FAR PASCAL APIDhaceUnArticulo)(char FAR *name);
typedef APIDhaceUnArticulo FAR *LPAPIDhaceUnArticulo;
#undef APIDhaceUnArticulo 
#define APIDhaceUnArticulo (*lpfnAPIDhaceUnArticulo) 
LPAPIDhaceUnArticulo lpfnAPIDhaceUnArticulo;
//*/
}

#endif //SUB
