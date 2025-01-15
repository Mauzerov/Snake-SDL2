#include <assert.h>

#include "image.h"

Direction _get_direction(Point * prev, Point * curr) {
    if (prev->x + 1 == curr->x && prev->y == curr->y)
        return Right;

    if (prev->x - 1 == curr->x && prev->y == curr->y)
        return Left;

    if (prev->y - 1 == curr->y && prev->x == curr->x)
        return Up;

    if (prev->y + 1 == curr->y && prev->x == curr->x)
        return Down;

    return Unknown;
}

void render_square_image(
    SDL_Renderer * renderer,
    Image * image,
    SDL_Rect * rect,
    int animation_frame,
    Direction direction
) {
    if (direction == Unknown)
        return;
    assert(image->height % image->width == 0 && "Image isn't a square nor a proper rectangle!");

    int frames = image->height / image->width;
    int frame  = animation_frame % frames;

    SDL_Rect src_rect = {
        0, frame * image->width,
        image->width, image->width
    };

    SDL_RenderCopyEx(
        renderer,
        image->texture,
        &src_rect,
        rect,
        90. * (direction),
        NULL,
        0
    );  
}

SDL_Texture * create_transparent_texture(
    SDL_Renderer * renderer,
    SDL_Surface * surface
) {
    // convert black to transparent
    SDL_SetColorKey(
        surface,
        SDL_TRUE,
        SDL_MapRGB(surface->format, 0, 0, 0)
    );
    SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (texture == NULL) {
        fprintf(stderr, "Could Not Initialize!: %d %s\n", __LINE__, SDL_GetError());
        exit(EXIT_FAILURE);
    }

    return texture;
}

SDL_Texture * create_texture(SDL_Renderer * renderer, char * path) {
    SDL_Surface * surface = SDL_LoadBMP(path);

    if (surface == NULL) {
        fprintf(stderr, "Could Not Initialize!: %d %s\n", __LINE__, SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Texture * texture = create_transparent_texture(renderer, surface);

    SDL_FreeSurface(surface);

    return texture;
}

Image * create_image(
    SDL_Renderer * renderer,
    SDL_Texture * texture,   // loaded bitmap (with removed black)
    SDL_Rect  rect,          // which part of bitmap to crop out
    SDL_Color color          // color to map white to
) {
    // Allow for negative width/height (get size)
    SDL_QueryTexture(
        texture, NULL, NULL,
        (rect.w < 0) ? &rect.w : NULL,
        (rect.h < 0) ? &rect.h : NULL
    );
    SDL_Texture * part_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        rect.w, rect.h
    );
    
    SDL_SetTextureBlendMode(part_texture, SDL_BLENDMODE_BLEND);
    // Clear `garbage pixels`
    SDL_SetRenderTarget    (renderer, part_texture);
    SDL_SetRenderDrawColor (renderer, 0, 0, 0, 0);
    SDL_RenderFillRect     (renderer, NULL);
    // Copy selected part onto a new texture with colors.
    SDL_SetTextureColorMod (texture, color.r, color.g, color.b);
    SDL_RenderCopy         (renderer, texture, &rect, NULL);

    Image * image  = malloc(sizeof(Image));
    image->width   = rect.w;
    image->height  = rect.h;
    image->texture = part_texture;
    // Reset Render Target
    SDL_SetRenderTarget(renderer, NULL);

    return image;
}

void destroy_image(Image * image) {
    if (image != NULL) {
        SDL_DestroyTexture(image->texture);
        free(image);
    }
}