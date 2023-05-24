#include <SDL2/SDL.h>
#define HEX_COLOR(hex)                                                  \
    ((hex) >> (3 * 8)) & 0xFF, ((hex) >> (2 * 8)) & 0xFF, ((hex) >> (1 * 8)) & 0xFF, ((hex) >> (0 * 8)) & 0xFF

void render_filled_circle(SDL_Renderer *renderer, SDL_Point pixel, int radius) {
    int offsetx, offsety, d;
    offsetx = 0;
    offsety = radius;
    d       = radius - 1;
    while (offsety >= offsetx) {
        SDL_RenderDrawLine(renderer, pixel.x - offsety, pixel.y + offsetx, pixel.x + offsety, pixel.y + offsetx);
        SDL_RenderDrawLine(renderer, pixel.x - offsetx, pixel.y + offsety, pixel.x + offsetx, pixel.y + offsety);
        SDL_RenderDrawLine(renderer, pixel.x - offsetx, pixel.y - offsety, pixel.x + offsetx, pixel.y - offsety);
        SDL_RenderDrawLine(renderer, pixel.x - offsety, pixel.y - offsetx, pixel.x + offsety, pixel.y - offsetx);

        if (d >= 2 * offsetx) {
            d -= 2 * offsetx + 1;
            offsetx += 1;
        } else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        } else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }
}

void render_filled_circle(SDL_Renderer *renderer, int x, int y, int radius) {
    SDL_Point pixel = {.x = x, .y = y};
    render_filled_circle(renderer, pixel, radius);
}

void render_thick_line(SDL_Renderer *renderer, SDL_Point begin_pixel, SDL_Point end_pixel, float width) {
    int x0 = begin_pixel.x;
    int y0 = begin_pixel.y;
    int x1 = end_pixel.x;
    int y1 = end_pixel.y;

    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err  = dx - dy, e2, x2, y2; /* error value e_xy */
    float ed = dx + dy == 0 ? 1 : sqrt((float)dx * dx + (float)dy * dy);

    for (width = (width + 1) / 2;;) { /* pixel loop */
        SDL_RenderDrawPoint(renderer, x0, y0);
        e2 = err;
        x2 = x0;
        if (2 * e2 >= -dx) { /* x step */
            for (e2 += dy, y2 = y0; e2 < ed * width && (y1 != y2 || dx > dy); e2 += dx) {
                SDL_RenderDrawPoint(renderer, x0, y2 += sy);
            }
            if (x0 == x1)
                break;
            e2 = err;
            err -= dy;
            x0 += sx;
        }
        if (2 * e2 <= dy) { /* y step */
            for (e2 = dx - e2; e2 < ed * width && (x1 != x2 || dx < dy); e2 += dy) {
                SDL_RenderDrawPoint(renderer, x2 += sx, y0);
            }
            if (y0 == y1)
                break;
            err += dx;
            y0 += sy;
        }
    }
}

int aaFilledEllipseRGBA(SDL_Renderer * renderer, float cx, float cy, float rx, float ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	int n, xi, yi, result = 0 ;
	double s, v, x, y, dx, dy ;

	if ((rx <= 0.0) || (ry <= 0.0))
		return -1 ;

	result |= SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) ;
	if (rx >= ry)
	    {
		n = ry + 1 ;
		for (yi = cy - n - 1; yi <= cy + n + 1; yi++)
		    {
			if (yi < (cy - 0.5))
				y = yi ;
			else
				y = yi + 1 ;
			s = (y - cy) / ry ;
			s = s * s ;
			x = 0.5 ;
			if (s < 1.0)
			    {
				x = rx * sqrt(1.0 - s) ;
				if (x >= 0.5)
				    {
					result |= SDL_SetRenderDrawColor (renderer, r, g, b, a ) ;
					result |= SDL_RenderDrawLine(renderer, cx - x + 1, yi, cx + x - 1, yi) ;
				    }
			    }
			s = 8 * ry * ry ;
			dy = fabs(y - cy) - 1.0 ;
			xi = cx - x ; // left
			while (1)
			    {
				dx = (cx - xi - 1) * ry / rx ;
				v = s - 4 * (dx - dy) * (dx - dy) ;
				if (v < 0) break ;
				v = (sqrt(v) - 2 * (dx + dy)) / 4 ;
				if (v < 0) break ;
				if (v > 1.0) v = 1.0 ;
				result |= SDL_SetRenderDrawColor (renderer, r, g, b, (double)a * v) ;
				result |= SDL_RenderDrawPoint (renderer, xi, yi) ;
				xi -= 1 ;
			    }
			xi = cx + x ; // right
			while (1)
			    {
				dx = (xi - cx) * ry / rx ;
				v = s - 4 * (dx - dy) * (dx - dy) ;
				if (v < 0) break ;
				v = (sqrt(v) - 2 * (dx + dy)) / 4 ;
				if (v < 0) break ;
				if (v > 1.0) v = 1.0 ;
				result |= SDL_SetRenderDrawColor (renderer, r, g, b, (double)a * v) ;
				result |= SDL_RenderDrawPoint (renderer, xi, yi) ;
				xi += 1 ;
			    }
		    }
	    }
	else
	    {
		n = rx + 1 ;
		for (xi = cx - n - 1; xi <= cx + n + 1; xi++)
		    {
			if (xi < (cx - 0.5))
				x = xi ;
			else
				x = xi + 1 ;
			s = (x - cx) / rx ;
			s = s * s ;
			y = 0.5 ;
			if (s < 1.0)
			    {
				y = ry * sqrt(1.0 - s) ;
				if (y >= 0.5)
				    {
					result |= SDL_SetRenderDrawColor (renderer, r, g, b, a ) ;
					result |= SDL_RenderDrawLine (renderer, xi, cy - y + 1, xi, cy + y - 1) ;
				    }
			    }
			s = 8 * rx * rx ;
			dx = fabs(x - cx) - 1.0 ;
			yi = cy - y ; // top
			while (1)
			    {
				dy = (cy - yi - 1) * rx / ry ;
				v = s - 4 * (dy - dx) * (dy - dx) ;
				if (v < 0) break ;
				v = (sqrt(v) - 2 * (dy + dx)) / 4 ;
				if (v < 0) break ;
				if (v > 1.0) v = 1.0 ;
				result |= SDL_SetRenderDrawColor (renderer, r, g, b, (double)a * v) ;
				result |= SDL_RenderDrawPoint (renderer, xi, yi) ;
				yi -= 1 ;
			    }
			yi = cy + y ; // bottom
			while (1)
			    {
				dy = (yi - cy) * rx / ry ;
				v = s - 4 * (dy - dx) * (dy - dx) ;
				if (v < 0) break ;
				v = (sqrt(v) - 2 * (dy + dx)) / 4 ;
				if (v < 0) break ;
				if (v > 1.0) v = 1.0 ;
				result |= SDL_SetRenderDrawColor (renderer, r, g, b, (double)a * v) ;
				result |= SDL_RenderDrawPoint (renderer, xi, yi) ;
				yi += 1 ;
			    }
		    }
	    }
	return result ;
}
