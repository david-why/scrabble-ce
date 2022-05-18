#ifndef __INT24_TYPE__
#define __INT24_TYPE__ int
#define __INT24_MAX__ 0x7fffff
#define __INT24_MIN__ (~__INT24_MAX__)
typedef __INT24_TYPE__ int24_t;
#define __UINT24_TYPE__ unsigned int
#define __UINT24_MAX__ 0xffffff
typedef __UINT24_TYPE__ uint24_t;
#endif

#include <graphx.h>
#include <tice.h>
#include <stdio.h>
#include <string.h>

#define TI_STRING_TYPE 0x04
#define ti_Ans ("\x72\0")

#define MAX_PLAYERS 4
#define COLOR_2WS 251
#define COLOR_3WS 233
#define COLOR_2LS 62
#define COLOR_3LS 28
#define COLOR_BORDER 148
#define COLOR_SEL 71

#define GAMEDATA_MOVED (1 << 0)
#define GAMEDATA_MULTICALC (1 << 1)

#pragma pack(push, 1)
typedef struct
{
    uint16_t _str_len;
    uint8_t players;
    uint8_t cur_player;
    uint8_t board[15][15];
    uint16_t scores[MAX_PLAYERS];
    uint8_t ntiles;
    uint8_t tiles[28];
    uint8_t player_tiles[MAX_PLAYERS][7];
    uint8_t empty[2];
    uint8_t flag;
} gamedata_t;
#pragma pack(pop)

sk_key_t waitkey()
{
    sk_key_t key;
    while (!(key = os_GetCSC()))
        delay(10);
    return key;
}

const uint8_t POINTS[] = {1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 5, 1, 3, 1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10, 0, 0};
const uint8_t TILES[] = {9, 2, 2, 4, 12, 2, 3, 2, 9, 1, 1, 4, 2, 6, 8, 2, 1, 6, 4, 6, 4, 2, 2, 1, 2, 1, 1, 1};

char buf[256];
uint8_t rsel = 7, csel = 7;

bool is_2ws(uint8_t r, uint8_t c)
{
    return (r == c || r == 14 - c) && ((r > 0 && r < 5) || (r == 7) || (r > 9 && r < 14));
}
bool is_3ws(uint8_t r, uint8_t c)
{
    return r % 7 == 0 && c % 7 == 0 && !(r == 7 && c == 7);
}
bool is_2ls(uint8_t r, uint8_t c)
{
    return ((r == 0 || r == 14) && (c == 3 || c == 11)) ||
           ((r == 2 || r == 12) && (c == 6 || c == 8)) ||
           ((r == 3 || r == 11) && (c == 0 || c == 7 || c == 14)) ||
           ((r == 6 || r == 8) && (c == 2 || c == 6 || c == 8 || c == 12)) ||
           (r == 7 && (c == 3 || c == 11));
}
bool is_3ls(uint8_t r, uint8_t c)
{
    return r % 4 == 1 && c % 4 == 1 && ((r == 1) + (r == 13) + (c == 1) + (c == 13)) < 2;
}

char board2char(const gamedata_t &game, uint8_t bc)
{
    if (bc < 26)
        return 'A' + bc;
    return 'a' + game.empty[bc - 26];
}
char tile2char(uint8_t tc)
{
    if (tc < 26)
        return 'A' + tc;
    if (tc == 255)
        return ' ';
    return '?';
}

uint8_t randtile(const gamedata_t &game)
{
    if (!game.ntiles)
        return 255;
    uint8_t sel = randInt(1, game.ntiles) - 1;
    uint8_t cnt = 0, c;
    for (c = 0; cnt <= sel; c++)
        cnt += game.tiles[c];
    return c - 1;
}

void distribute_tiles(gamedata_t &game)
{
    for (uint8_t p = 0; p < game.players; p++)
    {
        for (uint8_t n = 0; n < 7; n++)
        {
            uint8_t c = randtile(game);
            game.player_tiles[p][n] = c;
            game.tiles[c]--;
            game.ntiles--;
        }
    }
}

gamedata_t new_game()
{
    gamedata_t game;
    game._str_len = sizeof(game) - 2;
    os_ClrHome();
    while (true)
    {
        os_GetStringInput((char *)"Players?", buf, 3);
        puts("");
        real_t r = os_StrToReal(buf, NULL);
        game.players = os_RealToInt24(&r);
        if (game.players > MAX_PLAYERS || game.players < 1)
            puts("Invalid number of players");
        else
            break;
    }
    puts("Multi-calc mode?");
    puts("clear=no, other=yes");
    sk_key_t key;
    while (!(key = os_GetCSC()))
        delay(10);
    game.flag = key == sk_Clear ? 0 : GAMEDATA_MULTICALC;
    // if (key != sk_Clear)
    //     game.flag |= GAMEDATA_MULTICALC;
    game.cur_player = 0;
    memset(game.board, -1, sizeof(game.board));
    memset(game.scores, 0, sizeof(game.scores));
    game.ntiles = 100;
    memcpy(game.tiles, TILES, sizeof(TILES));
    puts("Distributing tiles...");
    distribute_tiles(game);
    return game;
}

void draw_screen(const gamedata_t &game)
{
    gfx_FillScreen(255);
    for (uint8_t r = 0; r < 15; r++)
        for (uint8_t c = 0; c < 15; c++)
        {
            uint16_t x = c * 16, y = r * 16;
            if (is_2ws(r, c))
                gfx_SetColor(COLOR_2WS), gfx_FillRectangle_NoClip(x + 1, y + 1, 14, 14);
            else if (is_3ws(r, c))
                gfx_SetColor(COLOR_3WS), gfx_FillRectangle_NoClip(x + 1, y + 1, 14, 14);
            else if (is_2ls(r, c))
                gfx_SetColor(COLOR_2LS), gfx_FillRectangle_NoClip(x + 1, y + 1, 14, 14);
            else if (is_3ls(r, c))
                gfx_SetColor(COLOR_3LS), gfx_FillRectangle_NoClip(x + 1, y + 1, 14, 14);
            if (rsel == r && csel == c)
                gfx_SetColor(COLOR_SEL), gfx_Rectangle_NoClip(x, y, 16, 16);
            if (game.board[r][c] != 255)
            {
                gfx_SetTextXY(x + 4, y + 4);
                if ((game.board[r][c] & 127) >= 26)
                    gfx_PrintChar(game.empty[(game.board[r][c] & 127) - 26] + 'a');
                else
                    gfx_PrintChar((game.board[r][c] & 127) + 'A');
            }
        }
    gfx_SetColor(0);
    gfx_Rectangle_NoClip(241, 1, 78, 238);
    gfx_PrintStringXY("Sco: ", 244, 20);
    gfx_PrintUInt(game.scores[game.cur_player], 3);
    // print_tiles(game, 244, 10);
    for (uint8_t i = 0; i < 7; i++)
    {
        gfx_SetTextXY(244 + 9 * i, 10);
        gfx_PrintChar(tile2char(game.player_tiles[game.cur_player][i]));
        // if (game.player_tiles[game.cur_player][i] == 255)
        //     ;
        // else if (game.player_tiles[game.cur_player][i] >= 26)
        //     gfx_PrintChar('?');
        // else
        //     gfx_PrintChar(game.player_tiles[game.cur_player][i] + 'A');
    }
    gfx_PrintStringXY("Action?", 244, 30);
    gfx_SwapDraw();
}

bool keymove(sk_key_t key)
{
    if (key == sk_Right)
    {
        csel++;
        if (csel == 15)
            csel = 0;
    }
    else if (key == sk_Left)
    {
        if (csel == 0)
            csel = 15;
        csel--;
    }
    else if (key == sk_Down)
    {
        rsel++;
        if (rsel == 15)
            rsel = 0;
    }
    else if (key == sk_Up)
    {
        if (rsel == 0)
            rsel = 15;
        rsel--;
    }
    else
        return false;
    return true;
}

enum tile_direction_t : uint8_t
{
    tile_direction_lr,
    tile_direction_ud
};

uint16_t calc_score(const gamedata_t &game, uint8_t r, uint8_t c, tile_direction_t dir)
{
    if (game.board[r][c] == 255)
        return 0;
    uint8_t wsm = 1;
    uint16_t sco = 0;
    if (dir == tile_direction_lr)
    {
        while (c > 0 && game.board[r][c - 1] != 255)
            c--;
        while (c < 15 && game.board[r][c] != 255)
        {
            uint8_t ls = POINTS[game.board[r][c] & 127];
            if (game.board[r][c] & 128)
            {
                if (is_3ls(r, c))
                    ls *= 3;
                else if (is_2ls(r, c))
                    ls *= 2;
                else if (is_2ws(r, c))
                    wsm *= 2;
                else if (is_3ws(r, c))
                    wsm *= 3;
            }
            sco += ls;
            c++;
        }
    }
    else
    {
        while (r > 0 && game.board[r - 1][c] != 255)
            r--;
        while (r < 15 && game.board[r][c] != 255)
        {
            uint8_t ls = POINTS[game.board[r][c] & 127];
            if (game.board[r][c] & 128)
            {
                if (is_3ls(r, c))
                    ls *= 3;
                else if (is_2ls(r, c))
                    ls *= 2;
                else if (is_2ws(r, c))
                    wsm *= 2;
                else if (is_3ws(r, c))
                    wsm *= 3;
            }
            sco += ls;
            r++;
        }
    }
    return sco * wsm;
}

void select_tile(gamedata_t &game)
{
    gfx_BlitScreen();
    gfx_SetTextXY(254, 40);
    gfx_PrintChar('A' + csel);
    gfx_PrintUInt(rsel + 1, 1);
    gfx_PrintStringXY("Direction?", 244, 50);
    gfx_BlitBuffer();
    tile_direction_t dir;
    while (true)
    {
        sk_key_t key;
        while (!(key = os_GetCSC()))
            delay(10);
        if (key == sk_Left || key == sk_Right)
            dir = tile_direction_lr;
        else if (key == sk_Up || key == sk_Down)
            dir = tile_direction_ud;
        else
            continue;
        break;
    }
    gfx_PrintStringXY((const char *[]){"L--R", "U--D"}[dir], 254, 60);
    gfx_PrintStringXY("Add/Sub?", 244, 70);
    gfx_PrintStringXY("Num: ", 244, 80);
    uint16_t sco = calc_score(game, rsel, csel, dir);
    gfx_PrintUInt(sco, 3);
    gfx_BlitBuffer();
    int16_t sgn;
    sk_key_t key;
    while (!(key = os_GetCSC()))
        delay(10);
    if (key == sk_Add)
        sgn = 1;
    else if (key == sk_Sub)
        sgn = -1;
    else
        return;
    game.scores[game.cur_player] += sgn * sco;
}

const char csc_chars[] = "\0\0\0\0\0\0\0\0\0\0\0WRMH\0\0\0\0VQLG\0\0\0ZUPKFC\0 YTOJEB\0\0XSNIDA\0\0\0\0\0\0\0\0";

void letter_input(gamedata_t &game, sk_key_t key)
{
    if (game.board[rsel][csel] != 255)
        return;
    char c = csc_chars[key];
    if (c == ' ')
        c = 26;
    else
        c -= 'A';
    uint8_t f = 255;
    for (uint8_t i = 0; i < 7; i++)
        if (game.player_tiles[game.cur_player][i] == c || (c == 26 && game.player_tiles[game.cur_player][i] == 27))
            f = i;
    if (f == 255)
        return;
    if (c == 26)
    {
        gfx_BlitScreen();
        gfx_PrintStringXY("Character?", 244, 50);
        gfx_BlitBuffer();
        while (true)
        {
            while (!(key = os_GetCSC()))
                delay(10);
            if (!csc_chars[key] || csc_chars[key] == ' ')
                continue;
            break;
        }
        game.empty[game.player_tiles[game.cur_player][f] - 26] = csc_chars[key] - 'A';
    }
    game.board[rsel][csel] = game.player_tiles[game.cur_player][f] | 128;
    game.player_tiles[game.cur_player][f] = 255;
    game.flag |= GAMEDATA_MOVED;
}

void end_turn(gamedata_t &game)
{
    gfx_BlitScreen();
    gfx_PrintStringXY("End turn", 254, 40);
    gfx_BlitBuffer();
    gfx_SetColor(255);
    if (!(game.flag & GAMEDATA_MOVED))
    {
        uint8_t exc = 0;
        while (true)
        {
            gfx_BlitScreen();
            gfx_PrintStringXY("Exchange:", 244, 50);
            gfx_FillRectangle_NoClip(243, 60, 75, 8);
            for (uint8_t i = 0; i < 7; i++)
                gfx_SetTextBGColor((exc & (1 << i)) ? 151 : 255),
                    gfx_SetTextXY(244 + 9 * i, 60),
                    gfx_PrintChar(tile2char(game.player_tiles[game.cur_player][i]));
            gfx_SetTextBGColor(255);
            gfx_BlitBuffer();
            sk_key_t key;
            while (!(key = os_GetCSC()))
                delay(10);
            if (key == sk_Enter)
                break;
            else if (key == sk_1)
                exc ^= (1 << 0);
            else if (key == sk_2)
                exc ^= (1 << 1);
            else if (key == sk_3)
                exc ^= (1 << 2);
            else if (key == sk_4)
                exc ^= (1 << 3);
            else if (key == sk_5)
                exc ^= (1 << 4);
            else if (key == sk_6)
                exc ^= (1 << 5);
            else if (key == sk_7)
                exc ^= (1 << 6);
        }
        for (uint8_t i = 0; i < 7; i++)
            if (exc & (1 << i))
            {
                game.tiles[game.player_tiles[game.cur_player][i]]++;
                game.player_tiles[game.cur_player][i] = 255;
            }
    }
    for (uint8_t i = 0; i < 7; i++)
        if (game.player_tiles[game.cur_player][i] == 255)
        {
            game.player_tiles[game.cur_player][i] = randtile(game);
            if (game.player_tiles[game.cur_player][i] != 255)
            {
                game.tiles[game.player_tiles[game.cur_player][i]]--;
                game.ntiles--;
            }
        }
    gfx_BlitScreen();
    gfx_FillRectangle_NoClip(242, 50, 76, 20);
    gfx_PrintStringXY("Tiles now:", 244, 50);
    for (uint8_t i = 0; i < 7; i++)
        gfx_SetTextXY(244 + 9 * i, 60),
            gfx_PrintChar(tile2char(game.player_tiles[game.cur_player][i]));
    gfx_BlitBuffer();
    waitkey();
    if (!(game.flag & GAMEDATA_MULTICALC))
    {
        gfx_FillScreen(255);
        gfx_SetTextScale(3, 3);
        gfx_PrintStringXY("Next!", 10, 10);
        gfx_SetTextScale(1, 1);
        gfx_SwapDraw();
        waitkey();
    }
    game.cur_player++;
    if (game.cur_player == game.players)
        game.cur_player = 0;
    for (uint8_t i = 0; i < 15; i++)
        for (uint8_t j = 0; j < 15; j++)
            if (game.board[i][j] != 255)
                game.board[i][j] = game.board[i][j] & 127;
    game.flag &= ~GAMEDATA_MOVED;
}

void del_tile(gamedata_t &game)
{
    uint8_t c = game.board[rsel][csel];
    if (!(c & 128))
        return;
    game.board[rsel][csel] = 255;
    c &= 127;
    for (uint8_t i = 0; i < 7; i++)
        if (game.player_tiles[game.cur_player][i] == 255)
        {
            game.player_tiles[game.cur_player][i] = c;
            return;
        }
}

int main()
{
    os_ClrHome();
    srand(rtc_Time());
    uint8_t ans_type;
    string_t *str = (string_t *)os_GetAnsData(&ans_type);
    gamedata_t game = *(gamedata_t *)str;
    if (ans_type != TI_STRING_TYPE || str->len != sizeof(gamedata_t) - 2)
    {
        puts("Variable Ans is not a string that contains game data");
        puts("");
        puts("Start new game?");
        puts("clear=no, other=yes");
        sk_key_t key;
        while (!(key = os_GetCSC()))
            delay(10);
        if (key == sk_Clear)
            return 0;
        game = new_game();
    }

    if (!(game.flag & GAMEDATA_MULTICALC))
        for (uint8_t p = 0; p < game.players; p++)
        {
            os_ClrHome();
            puts(p == 0 ? "Showing player data!" : "Next player!");
            waitkey();
            os_ClrHome();
            sprintf(buf, "Player #%u:", p + 1);
            puts(buf);
            puts("Tiles:");
            for (uint8_t i = 0; i < 7; i++)
                putchar(tile2char(game.player_tiles[p][i]));
            puts("\nScore:");
            sprintf(buf, "%u", game.scores[p]);
            puts(buf);
            waitkey();
        }

    gfx_Begin();
    gfx_SetDrawBuffer();
    while (true)
    {
        draw_screen(game);

        sk_key_t key;
        while (!(key = os_GetCSC()))
            delay(10);
        if (key == sk_Clear)
            break;
        if (key == sk_Enter)
            select_tile(game);
        else if (keymove(key))
            ;
        else if (csc_chars[key])
            letter_input(game, key);
        else if (key == sk_Mode)
        {
            end_turn(game);
            if (game.flag & GAMEDATA_MULTICALC)
                break;
        }
        else if (key == sk_Del)
            del_tile(game);
    }
    gfx_End();

    os_CreateString(ti_Ans, (string_t *)&game);

    return 0;
}
