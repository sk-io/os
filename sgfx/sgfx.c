#include "sgfx.h"

#define CLAMP(x, a, b) (((x) < (a)) ? (a) : ((x) > (b) ? (b) : (x)))
#define ABS(x) (((x) < 0) ? (-(x)) : (x))
#define MIN(x, y) (((x) < (y)) ?  (x) : (y))
#define MAX(x, y) (((x) > (y)) ?  (x) : (y))

void sgfx_init(GraphicsContext* ctx, u32* framebuffer, s32 width, s32 height) {
    ctx->framebuffer = framebuffer;
    ctx->width = width;
    ctx->height = height;
    ctx->bytesize = width * height * 4;
}

void sgfx_fill(const GraphicsContext* ctx, u32 color) {
    u32 size = ctx->bytesize / 4;

    for (u32 i = 0; i < size; i++) {
        ctx->framebuffer[i] = color;
    }
}

void sgfx_fill_rect(const GraphicsContext* ctx, s32 xdest, s32 ydest, s32 width, s32 height, u32 color) {
    if (xdest >= ctx->width)
        return;
    if (ydest >= ctx->height)
        return;
    if (xdest + width <= 0)
        return;
    if (ydest + height <= 0)
        return;

    s32 x0 = MAX(xdest, 0);
    s32 x1 = MIN(xdest + width, ctx->width);
    s32 y0 = MAX(ydest, 0);
    s32 y1 = MIN(ydest + height, ctx->height);

    for (s32 y = y0; y < y1; y++) {
        for (s32 x = x0; x < x1; x++) {
            ctx->framebuffer[x + y * ctx->width] = color;
        }
    }
}

void sgfx_copy_rect(const GraphicsContext* ctx, s32 xdest, s32 ydest, s32 width, s32 height, s32 xsrc, s32 ysrc, u32* source) {
    if (xdest >= ctx->width)
        return;
    if (ydest >= ctx->height)
        return;
    if (xdest + width <= 0)
        return;
    if (ydest + height <= 0)
        return;

    s32 x0 = MAX(xdest, 0);
    s32 x1 = MIN(xdest + width, ctx->width);
    s32 y0 = MAX(ydest, 0);
    s32 y1 = MIN(ydest + height, ctx->height);

    s32 yi = y0 - ydest + ysrc;
    for (s32 y = y0; y < y1; y++) {
        
        s32 xi = x0 - xdest + xsrc;
        for (s32 x = x0; x < x1; x++) {
            ctx->framebuffer[x + y * ctx->width] = source[xi + yi * width];

            xi++;
        }

        yi++;
    }
}

void sgfx_copy_rect_alpha(const GraphicsContext* ctx, s32 xdest, s32 ydest, s32 width, s32 height, s32 xsrc, s32 ysrc, u32* source) {
    if (xdest >= ctx->width)
        return;
    if (ydest >= ctx->height)
        return;
    if (xdest + width <= 0)
        return;
    if (ydest + height <= 0)
        return;

    s32 x0 = MAX(xdest, 0);
    s32 x1 = MIN(xdest + width, ctx->width);
    s32 y0 = MAX(ydest, 0);
    s32 y1 = MIN(ydest + height, ctx->height);

    s32 yi = y0 - ydest + ysrc;
    for (s32 y = y0; y < y1; y++) {
        
        s32 xi = x0 - xdest + xsrc;
        for (s32 x = x0; x < x1; x++) {
            u32 c = source[xi + yi * width];

            if ((c >> 24) != 0) {
                ctx->framebuffer[x + y * ctx->width] = c;
            }

            xi++;
        }

        yi++;
    }
}

#include "sgfx_font.inc"
#define READ_BIT(bitmap, index) (bitmap[((u32) index) / 8] & (0x80 >> (((u32) index) % 8)))

void sgfx_draw_char(const GraphicsContext* ctx, u8 c, s32 x0, s32 y0, u32 color) {
    s32 char_x = (s32) (c & 0xF) * 10;
    s32 char_y = (s32) (c >> 4) * 10;

    for (int y = 0; y < 10; y++) {
        int sy = y0 + y;
        if (sy < 0 || sy >= (s32) ctx->height)
            continue;

        for (int x = 0; x < 10; x++) {
            int sx = x0 + x;
            if (sx < 0 || sx >= (s32) ctx->width)
                continue;

            int iy = y + char_y;
            int ix = x + char_x;
            u8 b = READ_BIT(sgfx_font_bitmap, (ix + iy * 160));

            if (!b)
                continue;
            
            ctx->framebuffer[sx + sy * ctx->width] = color;
        }
    }
}

void sgfx_draw_string(const GraphicsContext* ctx, const char* str, s32 x0, s32 y0, u32 color) {
    char* c = str;

    s32 x = x0;
    s32 y = y0;
    while (*c != '\0') {
        if (*c != ' ')
            sgfx_draw_char(ctx, *c, x, y, color);

        c++;
        x += 10;
    }
}

void sgfx_draw_hline(const GraphicsContext* ctx, s32 x, s32 y, s32 w, u32 color) {
    if (w <= 0)
        return;
    if (y < 0 || y >= ctx->height)
        return;
    
    s32 x0 = x;
    s32 x1 = x + w;

    x0 = CLAMP(x0, 0, ctx->width);
    x1 = CLAMP(x1, 0, ctx->width);

    if (x1 - x0 <= 0)
        return;

    s32 i = x0 + y * ctx->width;
    s32 d = i + x1 - x0;

    while (i < d)
        ctx->framebuffer[i++] = color;
}

void sgfx_draw_vline(const GraphicsContext* ctx, s32 x, s32 y, s32 h, u32 color) {
    if (h <= 0)
        return;
    if (x < 0 || x >= ctx->width)
        return;
    
    s32 y0 = y;
    s32 y1 = y + h;

    y0 = CLAMP(y0, 0, ctx->height);
    y1 = CLAMP(y1, 0, ctx->height);

    if (y1 - y0 <= 0)
        return;

    s32 i = x + y0 * ctx->width;
    s32 d = i + (y1 - y0) * ctx->width;

    while (i < d) {
        ctx->framebuffer[i] = color;
        i += ctx->width;
    }
}

void sgfx_draw_box(const GraphicsContext* ctx, s32 x, s32 y, s32 width, s32 height, u32 color, u32 border_color) {
    sgfx_fill_rect(ctx, x + 1, y + 1, width - 2, height - 2, color);
    sgfx_draw_hline(ctx, x, y, width, border_color);
    sgfx_draw_hline(ctx, x, y + height - 1, width, border_color);
    sgfx_draw_vline(ctx, x, y, height, border_color);
    sgfx_draw_vline(ctx, x + width - 1, y, height, border_color);
}

void sgfx_pixel(GraphicsContext* context, s32 x, s32 y, u32 color) {
    if (x < 0) return;
    if (y < 0) return;
    if (x >= context->width) return;
    if (y >= context->height) return;
    context->framebuffer[x + y * context->width] = color;
}

void sgfx_draw_line(GraphicsContext* context, s32 x0, s32 y0, s32 x1, s32 y1, u32 color) {
    int dx =  ABS(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -ABS(y1 - y0), sy = y0 < y1 ? 1 : -1; 
    int err = dx + dy, e2;

    for (int i = 0; i < 1000; i++) {
        sgfx_pixel(context, x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;
        
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}
