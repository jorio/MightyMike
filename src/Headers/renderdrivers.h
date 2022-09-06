#pragma once

void GLRender_Init(void);
void GLRender_Shutdown(void);
void GLRender_PresentFramebuffer(void);

Boolean SDLRender_Init(void);
void SDLRender_Shutdown(void);
void SDLRender_InitTexture(void);
void SDLRender_PresentFramebuffer(void);
