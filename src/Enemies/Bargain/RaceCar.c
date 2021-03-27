/****************************/
/*      RACECAR             */
/* (c)1994 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/****************************/
/*    EXTERNALS             */
/****************************/
#include "myglobals.h"
#include "racecar.h"
#include "sound2.h"
#include "objecttypes.h"
#include "shape.h"
#include "misc.h"
#include "object.h"
#include "playfield.h"
#include "enemy.h"
#include "weapon.h"
#include "io.h"
#include "infobar.h"
#include "externs.h"

/****************************/
/*    CONSTANTS             */
/****************************/

struct TrackRec
{
	Byte	exitSide1,exitSide2;			// 0..7
	Byte	carAim;
	Byte	xData[32];
	Byte	yData[32];
};
typedef struct TrackRec TrackRec;


#define	TrackIndex		Special2
#define	CurrentTrack	Special3
#define	DirectionFlag	Flag0			//0=forward,1=reverse
#define	CarSpeed		Flag1

#define	CAR_RANGE		(PF_BUFFER_WIDTH*4)		// range of car

/**********************/
/*     VARIABLES      */
/**********************/

static const TrackRec	gTrackSegments[] =
{
						/* 0 */
	{
		AIM_RIGHT,AIM_LEFT,4,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27},
	},

						/* 1 */
	{
		AIM_DOWN_RIGHT,AIM_LEFT,4,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,28,28,29,29,30,30,30,31,31},
	},

						/* 2 */
	{
		AIM_RIGHT,AIM_UP_LEFT,5,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{ 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 8, 9,10,11},
	},

						/* 3 */
	{
		AIM_DOWN_RIGHT,AIM_LEFT,6,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{11,12,12,13,13,14,15,15,16,16,17,17,18,19,19,20,20,21,21,22,23,23,24,24,25,25,26,27,28,29,30,31},
	},

						/* 4 */
	{
		AIM_DOWN_RIGHT,AIM_UP_LEFT,7,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 5 */
	{
		AIM_DOWN,AIM_UP_LEFT,7,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,19,19,20,20,20},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 6 */
	{
		AIM_DOWN,AIM_UP,7,
		{20,20,21,21,21,22,22,22,23,23,23,24,24,24,25,25,25,26,26,26,27,27,27,28,28,28,28,29,29,29,29,29},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 7 */
	{
		AIM_DOWN,AIM_UP,8,
		{29,29,29,30,30,30,30,30,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 8 */
	{
		AIM_DOWN,AIM_UP,8,
		{31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 9 */
	{
		AIM_DOWN,AIM_UP,8,
		{31,31,31,31,31,31,31,31,31,31,31,31,31,30,30,30,29,29,28,28,27,27,26,26,25,25,24,24,24,23,23,23},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 10 */
	{
		AIM_DOWN,AIM_UP,9,
		{31,31,31,31,31,31,31,31,31,31,31,31,31,30,30,30,29,29,28,28,27,27,26,26,25,25,24,24,24,23,23,23},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 11 */
	{
		AIM_DOWN_LEFT,AIM_UP,9,
		{23,23,22,22,21,21,20,19,19,18,17,17,16,16,15,14,14,13,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 12 */

	{
		AIM_DOWN_LEFT,AIM_UP_RIGHT,10,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 13 */
	{
		AIM_LEFT,AIM_UP_RIGHT,11,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{ 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 7, 7, 8, 8, 9, 9,10,10,11,11,11,12,12,13,13,14,14},
	},

						/* 14 */
	{
		AIM_LEFT,AIM_RIGHT,11,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{14,14,14,14,15,15,15,15,16,16,16,16,17,17,17,17,18,18,18,18,19,19,19,19,20,20,20,20,21,21,21,21},
	},

						/* 15 */
	{
		AIM_LEFT,AIM_RIGHT,12,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{22,22,22,22,22,23,23,23,23,23,24,24,24,24,24,24,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25},
	},

						/* 16 */
	{
		AIM_LEFT,AIM_RIGHT,12,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25},
	},

						/* 17 */
	{
		AIM_LEFT,AIM_RIGHT,12,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,24,24,24,24,24,24,23,23,23,23,23,22,22,22,22,22},
	},

						/* 18 */
	{
		AIM_LEFT,AIM_RIGHT,13,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{21,21,21,21,20,20,20,20,19,19,19,19,18,18,18,18,17,17,17,17,16,16,16,16,15,15,15,15,14,14,14,14},
	},

						/* 19 */
	{
		AIM_UP_LEFT,AIM_RIGHT,14,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{14,14,13,13,12,12,11,11,11,10,10, 9, 9, 8, 8, 7, 7, 6, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0},
	},

						/* 20 */
	{
		AIM_UP_LEFT,AIM_DOWN_RIGHT,14,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 21 */
	{
		AIM_UP,AIM_DOWN_RIGHT,15,
		{31,30,30,29,29,28,28,27,27,26,26,25,25,24,24,23,23,22,22,21,21,20,20,19,19,18,18,17,17,16,16,15},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 22 */
	{
		AIM_UP,AIM_DOWN,15,
		{15,14,14,13,13,13,12,12,12,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 6, 5, 5, 5},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 23 */
	{
		AIM_UP,AIM_DOWN,0,
		{ 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 24 */
	{
		AIM_UP,AIM_DOWN,0,
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 25 */
	{
		AIM_UP,AIM_DOWN,0,
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 26 */
	{
		AIM_UP,AIM_DOWN,1,
		{ 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9,10,10,10,11,11,11,12,12,12,13,13,14,14,15,15,16},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 27 */
	{
		AIM_UP_RIGHT,AIM_DOWN,2,
		{16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,23,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 28 */
	{
		AIM_UP_RIGHT,AIM_DOWN_LEFT,2,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 29 */
	{
		AIM_RIGHT,AIM_DOWN_LEFT,3,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{31,30,29,28,27,26,25,24,23,22,21,21,20,19,18,17,16,16,15,14,13,13,12,11,10, 9, 9, 8, 8, 7, 7, 6},
	},

						/* 30 */
	{
		AIM_UP_RIGHT,AIM_LEFT,3,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{ 6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
	},

						/* 31 */
	{
		AIM_RIGHT,AIM_DOWN_LEFT,4,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{31,31,31,31,31,31,30,30,30,30,30,30,29,29,29,29,29,29,28,28,28,28,27,27,27,27,27,27,27,27,27,27},
	},

						/* 32 */
	{
		AIM_RIGHT,AIM_LEFT,4,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27},
	},

						/* 33 */
	{
		AIM_DOWN_RIGHT,AIM_LEFT,5,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,28,28,29,29,30,30,30,31,31},
	},

						/* 34 */
	{
		AIM_DOWN_RIGHT,AIM_UP_LEFT,6,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 35 */
	{
		AIM_DOWN,AIM_UP_LEFT,7,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,26,27,27,28,28},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 36 */
	{
		AIM_DOWN,AIM_UP,8,
		{29,29,29,30,30,30,30,30,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 37 */
	{
		AIM_DOWN,AIM_UP,8,
		{31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 38 */
	{
		AIM_DOWN,AIM_UP,8,
		{31,31,31,31,31,31,31,31,31,31,31,31,31,30,30,30,29,29,28,28,27,27,26,26,25,25,24,24,24,23,23,23},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 39 */
	{
		AIM_DOWN_LEFT,AIM_UP,9,
		{23,23,22,22,21,21,20,19,19,18,17,17,16,16,15,14,14,13,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 40 */
	{
		AIM_LEFT,AIM_UP_RIGHT,10,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{ 0, 1, 2, 3, 3, 4, 5, 6, 7, 8, 8, 9,10,10,11,12,12,13,14,15,16,16,17,18,18,19,20,20,21,21,22,22},
	},

						/* 41 */
	{
		AIM_LEFT,AIM_RIGHT,11,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{22,22,22,22,22,23,23,23,23,23,24,24,24,24,24,24,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25},
	},

						/* 42 */
	{
		AIM_LEFT,AIM_RIGHT,12,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25},
	},

						/* 43 */
	{
		AIM_LEFT,AIM_RIGHT,12,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,24,24,24,24,24,24,23,23,23,23,23,22,22,22,22,22},
	},

						/* 44 */
	{
		AIM_UP_LEFT,AIM_RIGHT,13,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{22,22,22,21,21,21,21,20,20,20,19,19,18,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 45 */
	{
		AIM_UP_LEFT,AIM_DOWN_RIGHT,14,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 46 */
	{
		AIM_UP,AIM_DOWN_RIGHT,15,
		{31,31,31,30,30,30,30,29,29,29,29,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 47 */
	{
		AIM_UP,AIM_DOWN,0,
		{28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 48 */
	{
		AIM_UP_RIGHT,AIM_DOWN,0,
		{28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,29,29,29,29,29,30,30,30,30,30,31,31,31,31},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 49 */
	{
		AIM_UP_RIGHT,AIM_DOWN_LEFT,1,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 50 */
	{
		AIM_UP_RIGHT,AIM_DOWN_LEFT,2,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 51 */
	{
		AIM_RIGHT,AIM_DOWN_LEFT,3,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{31,31,30,30,30,29,29,29,28,28,28,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27},
	},

//==================

						/* 52 */
	{
		AIM_UP_RIGHT,AIM_DOWN_RIGHT,0,
		{31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31},
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
	},

						/* 53 */
	{
		AIM_DOWN_LEFT,AIM_UP_LEFT,8,
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
	},

						/* 54 */
	{
		AIM_UP_RIGHT,AIM_UP_LEFT,4,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31},
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},

						/* 55 */
	{
		AIM_UP_LEFT,AIM_UP_RIGHT,12,
		{31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	},
};


Byte	gCarSpeeds[] = {12,4,6,8,10,12,14,16,18,20,22,24,26,28};

#define kNumTrackSegments ( (int)(sizeof(gTrackSegments)/sizeof(gTrackSegments[0])) )
#define kNumCarSpeeds ( (int)(sizeof(gCarSpeeds)/sizeof(gCarSpeeds[0])) )


/************************ ADD RACECAR ********************/

Boolean AddRaceCar(ObjectEntryType *itemPtr)
{
register	ObjNode		*newObj;
TileAttribType	*newTile;


	newObj = MakeNewShape(GroupNum_RaceCar,ObjType_RaceCar,0,
					itemPtr->x,itemPtr->y,40,MoveRaceCar,PLAYFIELD_RELATIVE);

	if (newObj == nil)
		return(false);

	newObj->ItemIndex = itemPtr;							// remember where this came from

	newObj->CType = CTYPE_ENEMYC;
	newObj->CBits = CBITS_TOUCHABLE;

	newObj->TopOff = -8;									// set box
	newObj->BottomOff = 8;
	newObj->LeftOff = -8;
	newObj->RightOff = 8;

	newObj->TileMaskFlag = false;							// doesnt use tile masks

	GAME_ASSERT_MESSAGE(itemPtr->parm[0] >= 0 && itemPtr->parm[0] < kNumCarSpeeds, "Car speed (parm[0]) out of range");

	newObj->TrackIndex = 0;									// init index into current track table
	newObj->DirectionFlag = 0;								// go forward
	newObj->CarSpeed = gCarSpeeds[itemPtr->parm[0]];		// set car speed

	newObj->BaseX = newObj->X.Int&0b1111111111100000;		// round down to nearest tile
	newObj->BaseY = newObj->Y.Int&0b1111111111100000;

	newTile = GetFullMapTileAttribs(newObj->BaseX,newObj->BaseY);	// use attribs to determine start
	GAME_ASSERT_MESSAGE(newTile->bits & TILE_ATTRIB_TRACK, "A RaceCar is not starting on a valid track piece! - fix it!");
	GAME_ASSERT_MESSAGE(newTile->parm0 >= 0 && newTile->parm0 < kNumTrackSegments, "Tile Track # out of range");

	newObj->CurrentTrack = newTile->parm0;					// set track piece #
	SwitchAnim(newObj,gTrackSegments[newObj->CurrentTrack].carAim);	// get aim

	return(true);
}


/******************** MOVE RACE CAR ****************************/

void MoveRaceCar(void)
{
short	trackIndex,currentTrack,oldAim,newAim;
short	xOff,yOff,oldX,oldY;
static	int nextOffsetsY[8] = {-TILE_SIZE,-TILE_SIZE,0,TILE_SIZE,TILE_SIZE,TILE_SIZE,0,-TILE_SIZE};
static	int nextOffsetsX[8] = {0,TILE_SIZE,TILE_SIZE,TILE_SIZE,0,-TILE_SIZE,-TILE_SIZE,-TILE_SIZE};
TileAttribType	*newTile;
static	int aimXlate[16] = {8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7};



				/* SEE IF REALLY FAR AWAY */

	if (((Absolute(gMyX-gThisNodePtr->X.Int) > CAR_RANGE)) ||
		(Absolute(gMyY-gThisNodePtr->Y.Int) > CAR_RANGE))
	{
		DeleteObject(gThisNodePtr);
		return;
	}

						/* MOVE THRU TRACK */

	trackIndex = gThisNodePtr->TrackIndex;									// get current info
	currentTrack = gThisNodePtr->CurrentTrack;

	if ((trackIndex += gThisNodePtr->CarSpeed) >= TILE_SIZE)				// move & see if @ end of track data
	{
		trackIndex -= TILE_SIZE;

		if (!gThisNodePtr->DirectionFlag)
		{
			gThisNodePtr->BaseX += nextOffsetsX[gTrackSegments[currentTrack].exitSide1]; // move to next tile
			gThisNodePtr->BaseY += nextOffsetsY[gTrackSegments[currentTrack].exitSide1];
		}
		else
		{
			gThisNodePtr->BaseX += nextOffsetsX[gTrackSegments[currentTrack].exitSide2]; // move to next tile reverse
			gThisNodePtr->BaseY += nextOffsetsY[gTrackSegments[currentTrack].exitSide2];
		}

		newTile = GetFullMapTileAttribs(gThisNodePtr->BaseX,gThisNodePtr->BaseY);	// see what's @ the new spot

		if (newTile->bits&TILE_ATTRIB_TRACK)								// make sure its a track piece
		{
			oldAim = gTrackSegments[currentTrack].carAim;					// remember old aim
			currentTrack = newTile->parm0;									// see which track part it is
			newAim = gTrackSegments[currentTrack].carAim;					// get new aim

			if (Absolute(oldAim-newAim) > 1) 								// see if reverse traverse
			{
				if ((oldAim==15)&&(newAim==0))								// check wrap cases
					goto no_reverse;
				if ((newAim==15)&&(oldAim==0))
					goto no_reverse;

				gThisNodePtr->DirectionFlag ^= 1;							// change direction
			}
no_reverse:
			if (!gThisNodePtr->DirectionFlag)
				SwitchAnim(gThisNodePtr,newAim);							// set new sprite aim
			else
				SwitchAnim(gThisNodePtr,aimXlate[newAim]);					// set new sprite aim reverse
		}
		else
					/* LANDED ON NON-TRACK PIECE */
		{


		}
	}

				/* GET COORD OFFSETS */

	if (!gThisNodePtr->DirectionFlag)						// see if going forward
	{
		xOff = gTrackSegments[currentTrack].xData[trackIndex];	// get x offset
		yOff = gTrackSegments[currentTrack].yData[trackIndex];	// get y offset
	}
	else
	{
		xOff = gTrackSegments[currentTrack].xData[31-trackIndex];	// get reverse data
		yOff = gTrackSegments[currentTrack].yData[31-trackIndex];
	}

				/* CALC NEW COORDS */

	GetObjectInfo();

	oldX = gX.Int;
	oldY = gY.Int;

	gX.Int = gThisNodePtr->BaseX+xOff;						// calc new X
	gY.Int = gThisNodePtr->BaseY+yOff;						// calc new y

	gDX = (long)(gX.Int-oldX)*0x10000L;						// calc deltas
	gDY = (long)(gY.Int-oldY)*0x10000L;

	gThisNodePtr->TrackIndex = trackIndex;					// update info
	gThisNodePtr->CurrentTrack = currentTrack;

	CalcObjectBox();
	UpdateObject();
}




