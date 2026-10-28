//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#pragma once

/**
*	@file
*
*	Common data types
*/

#if defined( _MSC_VER ) && _MSC_VER < 1920
// See _ENABLE_ATOMIC_ALIGNMENT_FIX comment in <atomic> header file to understand why "alignas( 8 )" is here
struct alignas( 8 ) Point
#else
struct Point
#endif
{
	int x = 0;
	int y = 0;
};

struct Rect
{
	int left = 0;
	int right = 0;
	int top = 0;
	int bottom = 0;
};
