# Mighty Mike Game Engine Information

v5/6/97

©1997 Pangea Software

All Rights Reserved

pangea@bga.com


# OVERVIEW

The Mighty Mike Game Engine is the code base and tools set which was used to create the game “Power Pete.”  The Tool set used to create data for this engine is in the application called OreoEdit and the documentation for the OreoEdit editor is contained in a separate document.

The Mighty Mike engine consists of several different sections which are covered in this document:

- Game Objects
- Sprites
- Playfields
- Audio

The code should be well documented in most places so much of the functionality should be self-explanatory.


# GAME OBJECTS

The first thing you need to learn about this game engine is that the entire thing essentially runs off of a gigantic linked list of “objects.”  An object is this engine’s way of representing everything from an enemy to a powerup to hidden triggers which activate things like teleporters.  The code which handles all of the basic functions dealing with this linked list is found in the file ObjectManager.c.

## What is an object?

An object is simply a node in a linked list.  The code often refers to objects as nodes and vice versa.  The structure which defines an object is called **ObjNode** (object node) and its definition as seen in Structures.h is as follows:

```c
struct ObjNode
{ 	
    long            Genre;           // obj genre: 0=sprite, 1=nonsprite
    unsigned long   Z;               // z sort value
    long            Type;            // obj type
    long            SubType;         // sub type (anim type)
    long            SpriteGroupNum;  // sprite group # (if sprite genre)
    Boolean         DrawFlag;        // set if draw this object
    Boolean         EraseFlag;       // set if erase this object
    Boolean         UpdateBoxFlag;   // set if automatically make update region...
    Boolean         MoveFlag;        // set if move this object
    Boolean         AnimFlag;        // set if animate this object
    Boolean         PFCoordsFlag;    // set if x/y coords are global playfield...
    Boolean         TileMaskFlag;    // set if PF draw should use tile masks
    union {
        long L;
        short Frac;
        short Int;
    } YOffset;                       // offset for y draw position on playfield
    short           ClipNum;         // clipping region # to use
    union {
        long L;
        short Frac;
        short Int;
    } X;                             // x coord (low word is fraction)
    union {
        long L;
        short Frac;
        short Int;
    } Y;                             // y coord (low word is fraction)
    long            OldX;            // old x coord (no fraction)
    long            OldY;            // old y coord
    Rect            drawBox;         // box obj was last drawn to
    long            DX;              // DX value (low word is fraction)
    long            DY;              // DY value
    long            DZ;              // DZ value
    void            (*MoveCall)(void);// pointer to object's move routine
    Ptr             AnimsList;       // ptr to object's animations list. nil = none
    long            AnimLine;        // line # in current anim
    long            CurrentFrame;    // current frame #
    unsigned long   AnimConst;       // default "setspeed" rate
    long            AnimCount;       // current value of rate
    unsigned long   AnimSpeed;       // amt to subtract from count/rate
    Boolean         Flag0;
    Boolean         Flag1;    
    Boolean         Flag2;    
    Boolean         Flag3;
    long            Special0;
    long            Special1;
    long            Special2;
    long            Special3;
    long            Misc1;
    long            Misc2;
    unsigned long   CType;           // collision type bits
    unsigned long   CBits;           // collision attribute bits
    long            LeftSide;        // collision side coords
    long            RightSide;
    long            TopSide;
    long            BottomSide;
    long            TopOff;          // collision box side offsets
    long            BottomOff;
    long            LeftOff;
    long            RightOff;
    long            Kind;            // kind
    long            BaseX;
    long            BaseY;
    long            Health;          // health
    Ptr             SHAPE_HEADER_Ptr;// addr of this sprite's SHAPE_HEADER
    long            OldLeftSide;
    long            OldRightSide;
    long            OldTopSide;
    long            OldBottomSide;
    ObjectEntryType *ItemIndex;      // pointer to item's spot in the ItemList
    struct ObjNode  *ShadowIndex;    // ptr to object's shadow...
    struct ObjNode  *OwnerToMessageNode;    // ptr to owner's message
    struct ObjNode  *MessageToOwnerNode;    // ptr to message's owner
    long            MessageTimer;    // time to display message

    long            Worth;           // "worth" of object / # coins to give
    long            InjuryThreshold; // threshold for weapon to do damage...
        
    long            NodeNum;         // node # in array (for internal use)
    struct ObjNode  *PrevNode;       // address of previous node in linked list
    struct ObjNode  *NextNode;       // address of next node in linked list
};
typedef struct ObjNode ObjNode;
```

The ObjNode structure is pretty massive, but it contains everything you’d ever need to know about any object in the entire game.  I’m not going to go into the gritty details of every single record in the ObjNode structure since the comments in the code explain most of it, but the fundamental ones will be covered in this document.

It is important to note that the linked list of ObjNodes is sorted from largest to smallest Z (the Z value being one of the records at the top of the ObjNode structure).  This list is sorted for several reasons:

- It allows events to happen in a desired order
- It makes sure that sprites in front of other sprites are drawn last.


## WORKING WITH OBJECTS

Creating a new object, adding it to the linked list, and removing it from the linked list are very simple tasks to do.  There are several functions to assist in this:

### MakeNewObject

This function is called to create a new object node and put it into the linked list.

```C
ObjNode    *MakeNewObject(
    Byte genre,
    short x,
    short y,
    unsigned short z,
    void *moveCall)
```

- `genre`:    This is the genre of object you wish to make.  The genre can be anything you like, but the types defined by the game engine are SPRITE_GENRE and BG_GENRE.  The former is for sprites, and the latter is for invisible background objects such as triggers. 
- `x & y`:    These are the playfield coordinate at which you want to put the object (if the object even needs coordinates.  Some objects are just events and thus don’t have actual locations in space).
- `z`:    This is the initial sorting value for the object - it’s where the new node is placed into the linked list.  The playfield engine will re-sort the sprites every frame to keep them properly aligned, thus their z’s will change.  This is ust the initial value to set the z to.
- `moveCall`:  This is a pointer to the object’s move handling routine.  The object manager will automatically call each object’s move routine every frame.  If the object doesn’t have a move routine, set this to nil.

MakeNewObject outputs a pointer to the new ObjNode which was just initialized and added to the linked list.


### MoveObjects

This function is called each frame to move all of the objects in the linked list.

```C
void MoveObjects(void)
```

MoveObjects scans thru the linked list of objects and calls each object’s move function (as set in the ObjNode’s MoveCall record).  But before calling the move routine, each object’s collision box and coordinate is copied into the “old” records.  The collision routines rely on knowing the previous location of an object in order to acurately handle collisions.

This function also handles the animation of all sprite genre objects.  If the object’s AnimFlag record is true, then AnimateASprite is called.

### DrawObjects

This function is called each frame (after MoveObjects) to draw all of the sprite genre objects in the linked list.

```C
void DrawObjects(void)
```

DrawObjects scans thru the linked list of objects and if the DrawFlag record is set then the sprite is drawn.

### EraseObjects

This function is called each frame to erase all of the sprite genre objects in the linked list which were recently drawn and displayed.

```C
void EraseObjects(void)
```

EraseObjects scans thru the linked list of objects and if the EraseFlag record is set then the sprite is erased.


### DeleteObject

This function is called to delete an object from the linked list.

```C
void DeleteObject(ObjNode *theNode)
```

- `theNode`:    A pointer to the ObjNode to delete. 


DeleteObject removes theNode from the linked list.  It sets the node’s CType field to INVALID_NODE_FLAG.  This is done to help prevent “double deletes” from happening.  The DeleteObject routine checks each input ObjNode and if it’s CType is set to INVALID_NODE_FLAG then an error is generated warning that the node has already been deleted.


The above are all of the basic object functions, but there are many other functions in ObjectManager.c which you can use to work with functions.


# SPRITE OBJECTS

Often, the code will refer to sprites as shapes.  Don’t worry, they’re the same thing.

To create a Sprite Object, it’s best not to call MakeNewObject directly, but rather to call the function:

### MakeNewShape

This function is called to create a new sprite object.

```C
ObjNode *MakeNewShape(
    long groupNum,
    long type,
    long subType,
    short x,
    short y,
    short z,
    void *moveCall,
    Boolean pfRelativeFlag)
```


- `groupNum`:     A sprite’s groupNum is the shape table number in which the sprite resides (see file.c). 
- `type`:    A sprite’s type is the sprite # within the group.
- `subType`:    The subType should really be called AnimNum since it is the animation number of the sprite to use.
- `pfRelativeFlag`:  This flag is set to true if the sprite is to be drawn onto a scrolling playfield.  Set it to false if it is to be drawn onto the screen.


MakeNewShape automatically creates a new ObjNode and initializes it as a sprite object.  It returns the new ObjNode which was created.


## ANIMATING SPRITE OBJECTS

As mentioned above, Sprites are one genre of objects.  An object is deemed to be a sprite if its Genre field is set to SPRITE_GENRE.  These sprite objects use most of the fields in the ObjNode structure.  They use all of the flags, coordinates, collision info, etc.

Sprites usually animate, and the animation sequences generated by OreoEdit can have special Set Flag commands embedded in them (see the OreoEdit documentation).  The animation handler in the game engine sets the records Flag0, Flag1, Flag2, and Flag3 when a flag command in the object’s animation is received.

The local flags are generally used to signal a particular event in the animation.  For example, if you have a throwing animation, you’d need to know the frame at which the ball leaves the player’s hand.  You could set a flag on this frame to tell the code.  Or a jump animation might use it since you’d need to know on which frame of the jump anim does the player actually leave the ground.

The Animation.c file contains all of the functions responsible for interpreting the animation sequences creating in OreoEdit.  The one function which you will be calling quite a bit is:

### SwitchAnim

This function is called to change a sprite object’s animation number.

```C
void SwitchAnim(
    ObjNode *theNodePtr,
    short animNum)
```

- `theNodePtr`:    A pointer to the ObjNode whose anim we want to change. 
- `animNum`:    The animation # (0..max) which we want to run.

SwitchAnim will reset the input sprite object’s animation by initializing the speed to 0x100 (1.0), the line number to 0, and the animation number to animNum.


The animation’s speed is controlled by the AnimSpeed field in the ObjNode structure.  Normally, this value is set to 0x100 which means play at 1.0x speed.  To play the animation at 2x speed, set it to 0x200.  Half speed would be 0x080.  One and a half would be 0x180.  The basic rule is bigger is faster.  This is opposite of the speedometer value set in the OreoEdit where bigger is slower.

So, if the player is just standing there (let’s say anim # MY_ANIM_STAND) and we want to start walking (let’s say anim # MY_ANIM_WALK), we’d do the following:

```C
SwitchAnim(myPlayerNode, MY_ANIM_WALK);
```

This will start the walk animation.  When the player stops moving, we’d do:

```C
SwitchAnim(myPlayerNode, MY_ANIM_STAND);
```


## DRAWING SPRITES

Sprites may be drawn in various modes.  Basically, a sprite is either drawn onto a scrolling playfield or onto the screen.  The PFCoordsFlag field in the ObjNode determines which to do (true == to playfield).

First, you should notice that there are 2 copies of the drawing routines in Shape.c.  One version compiles for 68000 machines and the other for PowerPC.

> NOTE:  The code assumes that it’s generating PowerPC code if it is compiled with Metrowerk’s CodeWarrior.  It checks `__MWERKS__`.  This is probably bad since you might be trying to complile the 68K version with CodeWarrior.  CodeWarrior does not allow the in-line assembly that is required to compiler the 68K code.  This can only be done with Think C which is now defunct.

Within the playfield and screen drawing routines are variations to do masked drawing, non-masked drawing, and other combinations.  The following describes each of the individual drawing routines:

### DrawSpriteAt

This function is called to draw a masked sprite to any given location in memory.  It could be a buffer, the screen, anything.

```C
void DrawSpriteAt(
    ObjNode *theNodePtr,
    long x,
    long y,
    Ptr destBufferBaseAddr,
    long rowBytes)
```

- `theNodePtr`:    A pointer to the sprite ObjNode to draw
- `x & y`:    The coordinates within the target buffer to draw.
- `destBufferBaseAddr`: Pointer to the draw buffer.
- `rowBytes`:     RowBytes value for the buffer.


### DrawFrameAt_NoMask

This function is called to draw a sprite frame with no mask to any given location in memory.  It could be a buffer, the screen, anything.

```C
void DrawFrameAt_NoMask (
    long x,
    long y,
    long groupNum,
    long shapeNum,
    long frameNum,
    Ptr destBufferBaseAddr,
    long rowBytes)
```

- `x & y`:    The coordinates within the target buffer to draw.
- `groupNum`:    Sprite group # which the frame resides in
- `shapeNum`:     Sprite # in the group to use.
- `frameNum`:     Frame # in the sprite to draw.
- `destBufferBaseAddr`: Pointer to the draw buffer.
- `rowBytes`:     RowBytes value for the buffer.


### DrawASprite

This is the main sprite drawing routine which is called from MoveObjects.  It recognizes whether a sprite needs to be drawn to the playfield or screen and does the appropriate action.

```C
void DrawASprite(
    ObjNode *theNodePtr)
```

- `theNodePtr`:    Pointer to the sprite ObjNode to draw.

> Note:  On the 68000 there is an additional “interlace” draw and erase option.  This is a special mode which only draws every other line if the global flag gInterlaceMode is set.  Interlacing is not available on the PowerPC version since it isn’t necessary.


### EraseASprite

This is the complement of DrawASprite.  It is the main routine for erasing sprites and will automatically handle playfield or screen sprites.

```C
void EraseASprite(
    ObjNode *theNodePtr)
```

- `theNodePtr`:    Pointer to the sprite ObjNode to erase.


Also note that there is a field in the ObjNode called ClipNum.  You can have multiple “windows” or clipping regions active in a game, and you can tell the engine which region you want to clip a sprite against when it is drawn.  The Power Pete code only uses 1 clipping region so there aren’t any good examples of this in the code.  Nonetheless, the region handling code is in ObjectManager.c. 

### AddUpdateRegion

This is used to define a rectangular clipping region and add it to the region list.

```C
void AddUpdateRegion(
    Rect theRegion,
    Byte clipNum)
```

- `theRegion`:    A rect defining the bounds of the region.
- `clipNum`:    The clipping region we are defining.  This should match the ClipNum field in the ObjNode structure to clip against this rect.



# THE PLAYFIELD

The playfield is a giant matrix of tiles which is created in OreoEdit’s map editor.  The game engine’s playfield code is fairly well self contained and the intricate details of it’s operation are not covered in this document.  Rather, this text focuses on the many routines which are used to control the scrolling of the playefield itself.  All of the playfield code is located in the files Playfield.c and TileAnim.c.

The size and coordinates of the playfield are determined by PF_TILE_HEIGHT, PF_TILE_WIDTH, PF_WINDOW_TOP, and PF_WINDOW_LEFT.

> Note:  For a 68K compile, these values are defined as constants in playfield.h, but for PowerPC compiles, these are defined as variables in Playfield.c.


### CreatePlayfieldPermanentMemory

This function must be called at boot time to allocate and initialize memory used by the playfield scrolling engine.

```C
void CreatePlayfieldPermanentMemory(void)
```

### LoadTileSet

This will load a playfield’s .tileset file.

```C
void LoadTileSet(Str255 fileName)
```

- `fileName`:    The name of the .tileset file to load.

### LoadPlayfield

This is the function which should be called immediately after LoadTileSet.  It will load a playfield’s .map file.

```C
void LoadPlayfield(Str255 fileName)
```

- `fileName`:    The name of the .map file to load.


### InitPlayfield

Once the tileset and map files have been loaded you call this function to initialize the playfield.  The global variables gMyX and gMyY must be set to the desired initial scrolling coordinates that the playfield will start at.

```C
void InitPlayfield(void)
```

InitPlayfield also will initialize any map items/objects which are in the initial viewing zone.


### DoMyScreenScroll

This is the first critical routine used to actually scroll the screen.  This function should be called when the main player (or whatever is controlling the scrolling) moves.

```C
void DoMyScreenScroll(void)
```

Several global variables must be set for this function to work correctly. gMyDirection is a value from 0 to 7 (clockwise) which represents one of 8 possible directions which the main player is aiming.  This value is used to smoothly adjust the “floating window” effect as the player moves around.  The floating window keeps the screen from scrolling in a jerky fashion, but rather in a floaty smooth fashion.

The variables gMySumDX and gMySumDY are critical.  They determine the speed that the screen should scroll and in which direction.

> Note:  You must be careful never to move more than 32 pixels in a single frame since the scroll engine can only scroll at a maximum speed of one tile per frame!  Otherwise, the player may outrun the scrolling playfield.


### ScrollPlayfield

This is the actual scrolling code.  It should be called after MoveObjects (which would have caused DoMyScreenScroll to have been called).

```C
void ScrollPlayfield(void)
```

The global variables gScrollX and gScrollY determine the current scroll position of the playfield.  These values are modified and set correctly by DoMyScreenScroll.

When the playfield is scrolled, this function scans for playfield items and automatically calls any found item’s init routine.  Every object type has an initialization routine pointer which is located in the jump table gItemAddPtrs which is at the top of playfield.c.

### StartShakeyScreen

When called, this will cause the scroll engine to “shake” the playfield randomly each frame.

```C
void StartShakeyScreen(
    short duration)
```

- `duration`:    # of frames the screen should shake



# COLLISION DETECTION

This section explains how the collision detection in the game engine works.  There are basically 2 different types of collision:

- Object collision
- Tile collision

The file Collision.c contains all of the core collision detection routines, but much of the custom handling is done in MyGuy.c and Enemy.c.  The additional collision code in those files handles the specifics of player and enemy collisions for both objects and tiles.


## OBJECT COLLISION

Object collision is the collision between two ObjNode’s.  There are several fields in the ObjNode structure which are important to the collision detecting process:

```
CType           :    “Collision Type” for use in classifying an object.
CBits           :    “Collision Bits” specifies collision options.
LeftSide        :    X coordinate of collision box’s left side.
RightSide       :    X coordinate of collision box’s right side.
BottomSide      :    Y coordinate of collision box’s bottom side.
TopSide         :    Y coordinate of collision box’s top side.
TopOff          :    y offset from object’s origin to top side
BottomOff       :    y offset from object’s origin to bottom side
LeftOff         :    x offset from object’s origin to left side
RightOff        :    x offset from object’s origin to right side
OldLeftSide     :    object’s left side from previous frame of animation.
OldRightSide    :    object’s right side from previous frame of animation.
OldTopSide      :    object’s top side from previous frame of animation.
OldBottomSide   :    object’s bottom side from previous frame of animation.
```

### CTYPE & CBITS

The CType field determines the collision classification of an object.  When a collision function is called a ctype value is passed in which indicates what objects to perform collision on.  An object is only tested if one or more of it’s CType bits match the input CType’s bits.  Many useful predefined CType values are found in Equates.h:

```C
enum
{
    CTYPE_MYGUY     = 1,         // %0000000000000001  Me
    CTYPE_ENEMYA    = (1L<<1),   // %0000000000000010  Enemy
    CTYPE_ENEMYB    = (1L<<2),   // %0000000000000100  Enemy projectile (stops when hits me)
    CTYPE_BONUS     = (1L<<3),   // %0000000000001000  Bonus item
    CTYPE_MYBULLET  = (1L<<4),   // %0000000000010000  My Bullet
    CTYPE_TRIGGER   = (1L<<5),   // %0000000000100000  Trigger
    CTYPE_BGROUND   = (1L<<6),   // %0000000001000000  BGround
    CTYPE_MISC      = (1L<<7),   // %0000000010000000  Misc
    CTYPE_ENEMYC    = (1L<<8),   // %0000000100000000  Enemy projectile (will go thru me) / generic harmful things
    CTYPE_HEALTH    = (1L<<9),   // %0000001000000000  MUST be combined with BONUS to form a health bonus
    CTYPE_MPLATFORM = (1L<<10),  // %0000010000000000  MPlatform
    CTYPE_KEY       = (1L<<11),  // %0000100000000000  Key Type
    CTYPE_WEAPONPOW = (1L<<12),  // %0001000000000000  Weapon Powerup
    CTYPE_MISCPOW   = (1L<<13),  // %0010000000000000  Misc Powerup
    CTYPE_HURTENEMY = (1L<<14)   // &0100000000000000  Misc hurt Enemy item
};
```


To define an object as  a Weapon powerup, the following CType is used.

```C
objNode->CType = CTYPE_WEAPONPOW;
```

In addition to the CType, the CBits must also be set.  These bits determine specific collision attributes and are also found in Equates.h.  The following code will make the object “touchable” which means that it is not a solid object.  The collision code will detect a hit, but will allow other objects to pass thru it.

```C
objNode->CBits = CBITS_TOUCHABLE;
```

To make the object entirely solid and impassable, the code would look like:

```C
objNode->CBits = CBITS_ALLSOLID;
```


### COLLISION BOX

Every object needs a collision box.  The collision box is a bounding box which closely approximates the bounds of an object.  If an object is moving, this bounding box will need to be updated on every frame.  To make this easy, we do the following:

```C
    newObj->TopOff     = -10;    
    newObj->BottomOff = 10;        
    newObj->LeftOff     = -10;        
    newObj->RightOff     = 10;        
```

This defines the size of the box in terms of offsets from the object’s origin.  The “origin” is the place in the sprite which actually represent’s the sprite’s coordinate.  It is the place defined by the crosshairs in OreoEdit when aligning the sprite frames.  To actually calculate the bounding box, one of 2 functions must be called:

```C
    CalcObjectBox2(newObj);
```

This will calculate the bounding box and store the results directly into the TopSide,BottomSide,LeftSide, & RightSide fields in the ObjNode.

```C
    CalcObjectBox();
```

This version calculates the bounding box for the object defined by gThisNodePtr and stores the result in the global variables gTopSide, gBottomSide, gLeftSide, & gRightSide.  

To insure accurate collision reporting, it is important to know the previous bounding box as well as the current bounding box of an object.  This provides information required to determine which side of which box passed thru which side of another box.  The function MoveObjects automatically makes a copy of the old collision box before calling an object’s move routine.

### PERFORMING COLLISIONS

The collision routines require that several global variables be set:

```
gRightSide  :    Must be set to the current right side of the object
gLeftSide   :    Must be set to the current left side of the object
gTopSide    :    Must be set to the current top side of the object
gBottomSide :    Must be set to the current bottom side of the object
gSumDX      :    Must be set to the object’s x velocity
gSumDY      :    Must be set to the object’s y velocity
```

The side variables are just the current bounding box values calculated by calling CalcObjectBox.

The gSumDX and gSumDY variables represent the total velocity of the object.  They are the object’s DX and DY plus any addition velocity (from say a moving platform).

Once these variables are set, there are many ways to perform and handle the collisions:

#### CollisionDetect

This is the lowest level collision routine you can call.  It performs all the collisions as indicated in CType and returns all of the results in the gCollisionList.  It does not handle the collision, it mearly builds a list and reports them.

```C
void CollisionDetect(
    ObjNode *baseNode,
    unsigned long CType)
```

- `baseNode`: the ObjNode to do collision
- `CType`: a CType defining all of the objects to collide against.


#### HandleCollisions

This function does generic collision handling.  It calls CollisionDetect (see above) and then handles the collision information returned in gCollisionList.  When a collision is found in the list, this function will handle all solid objects, but does not account for specific collisions such as triggers, bullets hitting enemies, etc.  The function does not take an input ObjType, but assumes it’s doing collision on the ObjNode defined by gThisNodePtr.

```C
Byte HandleCollisions(unsigned long cType)
```


#### DoMyCollisionDetect

This is a high-level collision handler routine for use in doing player collision detect.  It takes care of building a collision list and then handling all of the results including the special stuff like enemies, triggers, etc.  It also checks for special tile attribute collisions like death, hurt, water, etc. tiles.

```C
Boolean DoMyCollisionDetect(void)
```

#### DoEnemyCollisionDetect

Like DoMyCollisionDetect, this is another high-level collision routine for handling enemy collisions.

```C
Boolean DoEnemyCollisionDetect(unsigned long CType)
```

### TILE COLLISION

The above mentioned routines will handle tile collision detection as long as the CTYPE_BGROUND bit is set in the input CType.  The collision functions determine tile collision information from the tile’s collision bits  as set in OreoEdit.

