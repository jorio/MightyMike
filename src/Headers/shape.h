//
// shapes.h
//

#define	PLAYFIELD_RELATIVE	true
#define	SCREEN_RELATIVE		false

#pragma pack(push, 1)
typedef struct FrameHeader
{
	int16_t		width;
	int16_t		height;
	int16_t		x;
	int16_t		y;
	int32_t		pixelOffset;	// offset from shape header
	int32_t		maskOffset;		// offset from shape header
} FrameHeader;

typedef struct FrameList
{
	int16_t		numFrames;
	int32_t		frameHeaderOffsets[];
} FrameList;
#pragma pack(pop)

ObjNode	*MakeNewShape(long groupNum, long type, long subType, short x, short y, short z, void (*moveCall)(void), Boolean pfRelativeFlag);
void LoadShapeTable(const char* filename, long groupNum);
const FrameHeader* GetFrameHeader(long groupNum, long shapeNum, long frameNum, const uint8_t** outPixelPtr, const uint8_t** outMaskPtr);
void	DrawFrameToScreen(long, long, long, long, long);
void	DrawFrameToScreen_NoMask(long, long, long, long, long);
void DrawFrameToBackground(long x, long y, long groupNum, long shapeNum, long frameNum);
void	ZapShapeTable(long);
bool	CheckFootPriority(long x, long y, long width);
void	DrawASprite(ObjNode *);
void	EraseASprite(ObjNode *);
