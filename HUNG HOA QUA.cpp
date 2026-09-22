#include <windows.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>

using namespace std;

// ============================================================
// THƯ VIỆN WINDOWS
// ============================================================

#pragma comment(lib, "Msimg32.lib")
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")


// ============================================================
// KÍCH THƯỚC GAME
// ============================================================

const int GAME_WIDTH = 800;
const int GAME_HEIGHT = 600;


// ============================================================
// ID
// ============================================================

#define TIMER_GAME 1

#define BTN_PLAY 1001
#define BTN_SETTINGS 1002
#define BTN_EXIT 1003

#define BTN_BACK 1004
#define BTN_REPLAY 1005
#define BTN_MENU 1006
#define BTN_SOUND 1007


// ============================================================
// TRẠNG THÁI GAME
// ============================================================

enum GameState
{
    MENU,
    PLAYING,
    SETTINGS,
    GAME_OVER
};

GameState gameState = MENU;


// ============================================================
// ĐỐI TƯỢNG RƠI
// ============================================================

enum ObjectType
{
    FRUIT,
    BOMB
};

struct FallingObject
{
    float x;
    float y;

    float speed;

    int radius;

    ObjectType type;
};

vector<FallingObject> objects;


// ============================================================
// BIẾN GAME
// ============================================================

int score = 0;
int lives = 3;

int gameTime = 60;

int basketX = GAME_WIDTH / 2;
int basketY = 530;

int mouseX = GAME_WIDTH / 2;
int mouseY = GAME_HEIGHT / 2;

bool soundEnabled = true;

DWORD lastSpawnTime = 0;
DWORD lastSecondTime = 0;

int fruitColorIndex = 0;


// ============================================================
// CẤU TRÚC NÚT
// ============================================================

struct Button
{
    int id;

    int x;
    int y;

    int width;
    int height;

    string text;

    bool hover;
};

vector<Button> buttons;


// ============================================================
// HÀM TIỆN ÍCH
// ============================================================

bool IsInside(
    int x,
    int y,
    const Button& b
)
{
    return
        x >= b.x &&
        x <= b.x + b.width &&
        y >= b.y &&
        y <= b.y + b.height;
}


void PlaySoundEffect(
    int frequency,
    int duration
)
{
    if (soundEnabled)
    {
        Beep(frequency, duration);
    }
}


// ============================================================
// VẼ CHỮ
// ============================================================

void DrawTextCenter(
    HDC hdc,
    int x,
    int y,
    const string& text,
    int size,
    COLORREF color
)
{
    HFONT font = CreateFontA(
        size,
        0,
        0,
        0,
        FW_BOLD,
        FALSE,
        FALSE,
        FALSE,
        ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        "Arial"
    );

    HFONT oldFont =
        (HFONT)SelectObject(hdc, font);

    SetTextColor(hdc, color);

    SetBkMode(
        hdc,
        TRANSPARENT
    );

    RECT rect;

    rect.left = x - 300;
    rect.top = y - size;
    rect.right = x + 300;
    rect.bottom = y + size;

    DrawTextA(
        hdc,
        text.c_str(),
        -1,
        &rect,
        DT_CENTER |
        DT_VCENTER |
        DT_SINGLELINE
    );

    SelectObject(
        hdc,
        oldFont
    );

    DeleteObject(font);
}


// ============================================================
// VẼ TEXT GÓC TRÁI
// ============================================================

void DrawTextLeft(
    HDC hdc,
    int x,
    int y,
    const string& text,
    int size,
    COLORREF color
)
{
    HFONT font = CreateFontA(
        size,
        0,
        0,
        0,
        FW_BOLD,
        FALSE,
        FALSE,
        FALSE,
        ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        "Arial"
    );

    HFONT oldFont =
        (HFONT)SelectObject(hdc, font);

    SetTextColor(hdc, color);

    SetBkMode(
        hdc,
        TRANSPARENT
    );

    TextOutA(
        hdc,
        x,
        y,
        text.c_str(),
        (int)text.length()
    );

    SelectObject(
        hdc,
        oldFont
    );

    DeleteObject(font);
}


// ============================================================
// VẼ NỀN CHUYỂN MÀU
// ============================================================

void DrawBackground(HDC hdc)
{
    TRIVERTEX vertex[2];

    vertex[0].x = 0;
    vertex[0].y = 0;

    vertex[0].Red = 0x1900;
    vertex[0].Green = 0xB400;
    vertex[0].Blue = 0xFF00;
    vertex[0].Alpha = 0;

    vertex[1].x = GAME_WIDTH;
    vertex[1].y = GAME_HEIGHT;

    vertex[1].Red = 0x6400;
    vertex[1].Green = 0xE800;
    vertex[1].Blue = 0xFF00;
    vertex[1].Alpha = 0;

    GRADIENT_RECT rect;

    rect.UpperLeft = 0;
    rect.LowerRight = 1;

    GradientFill(
        hdc,
        vertex,
        2,
        &rect,
        1,
        GRADIENT_FILL_RECT_V
    );
}


// ============================================================
// VẼ NỀN MENU
// ============================================================

void DrawMenuBackground(HDC hdc)
{
    DrawBackground(hdc);

    // Vẽ các vòng tròn trang trí

    HBRUSH brush =
        CreateSolidBrush(
            RGB(120, 200, 255)
        );

    HBRUSH oldBrush =
        (HBRUSH)SelectObject(
            hdc,
            brush
        );

    Ellipse(
        hdc,
        50,
        50,
        120,
        120
    );

    Ellipse(
        hdc,
        680,
        80,
        750,
        150
    );

    Ellipse(
        hdc,
        100,
        470,
        160,
        530
    );

    Ellipse(
        hdc,
        650,
        450,
        720,
        520
    );

    SelectObject(
        hdc,
        oldBrush
    );

    DeleteObject(brush);
}


// ============================================================
// TẠO NÚT
// ============================================================

void CreateMenuButtons()
{
    buttons.clear();

    int width = 240;
    int height = 60;

    int x =
        (GAME_WIDTH - width) / 2;

    buttons.push_back(
        {
            BTN_PLAY,
            x,
            230,
            width,
            height,
            "CHOI NGAY",
            false
        }
    );

    buttons.push_back(
        {
            BTN_SETTINGS,
            x,
            310,
            width,
            height,
            "CAI DAT",
            false
        }
    );

    buttons.push_back(
        {
            BTN_EXIT,
            x,
            390,
            width,
            height,
            "THOAT",
            false
        }
    );
}


// ============================================================
// VẼ NÚT
// ============================================================

void DrawButton(
    HDC hdc,
    const Button& b
)
{
    RECT rect;

    rect.left = b.x;
    rect.top = b.y;
    rect.right = b.x + b.width;
    rect.bottom = b.y + b.height;

    COLORREF buttonColor;

    if (b.hover)
    {
        buttonColor =
            RGB(100, 190, 255);
    }
    else
    {
        buttonColor =
            RGB(50, 130, 220);
    }

    HBRUSH brush =
        CreateSolidBrush(
            buttonColor
        );

    FillRect(
        hdc,
        &rect,
        brush
    );

    DeleteObject(brush);


    // Viền

    HPEN pen =
        CreatePen(
            PS_SOLID,
            2,
            RGB(255, 255, 255)
        );

    HPEN oldPen =
        (HPEN)SelectObject(
            hdc,
            pen
        );

    HBRUSH oldBrush =
        (HBRUSH)SelectObject(
            hdc,
            GetStockObject(NULL_BRUSH)
        );

    Rectangle(
        hdc,
        b.x,
        b.y,
        b.x + b.width,
        b.y + b.height
    );

    SelectObject(
        hdc,
        oldPen
    );

    SelectObject(
        hdc,
        oldBrush
    );

    DeleteObject(pen);


    DrawTextCenter(
        hdc,
        b.x + b.width / 2,
        b.y + b.height / 2,
        b.text,
        23,
        RGB(255, 255, 255)
    );
}


// ============================================================
// VẼ MENU
// ============================================================

void DrawMenu(HDC hdc)
{
    DrawMenuBackground(hdc);


    DrawTextCenter(
        hdc,
        GAME_WIDTH / 2,
        90,
        "FRUIT CATCH GAME",
        44,
        RGB(255, 255, 255)
    );


    DrawTextCenter(
        hdc,
        GAME_WIDTH / 2,
        145,
        "HUNG TRAI CAY - TRANH BOM",
        19,
        RGB(230, 255, 200)
    );


    // Vẽ nút

    for (const auto& button : buttons)
    {
        DrawButton(
            hdc,
            button
        );
    }


    DrawTextCenter(
        hdc,
        GAME_WIDTH / 2,
        490,
        "Dung chuot de dieu khien gio",
        16,
        RGB(255, 255, 255)
    );


    DrawTextCenter(
        hdc,
        GAME_WIDTH / 2,
        520,
        "An trai cay +10 diem | Tranh bom!",
        16,
        RGB(255, 255, 255)
    );
}


// ============================================================
// KHỞI TẠO GAME
// ============================================================

void StartGame()
{
    gameState = PLAYING;

    score = 0;

    lives = 3;

    gameTime = 60;

    basketX =
        GAME_WIDTH / 2;

    mouseX =
        GAME_WIDTH / 2;

    objects.clear();

    lastSpawnTime =
        GetTickCount();

    lastSecondTime =
        GetTickCount();

    PlaySoundEffect(
        700,
        80
    );
}


// ============================================================
// TẠO TRÁI CÂY / BOM
// ============================================================

void SpawnObject()
{
    FallingObject object;

    object.x =
        (float)(30 + rand() % (GAME_WIDTH - 60));

    object.y = -40;

    object.speed =
        2.0f +
        (float)(rand() % 30) / 10.0f;

    object.radius =
        18 + rand() % 8;


    // Khoảng 20% là bom

    if (rand() % 5 == 0)
    {
        object.type = BOMB;
    }
    else
    {
        object.type = FRUIT;
    }


    objects.push_back(object);
}


// ============================================================
// VẼ TRÁI CÂY
// ============================================================

void DrawFruit(
    HDC hdc,
    const FallingObject& object
)
{
    int x = (int)object.x;
    int y = (int)object.y;

    int r = object.radius;


    COLORREF color;


    switch (fruitColorIndex % 4)
    {
    case 0:
        color = RGB(255, 80, 80);
        break;

    case 1:
        color = RGB(255, 180, 30);
        break;

    case 2:
        color = RGB(80, 210, 100);
        break;

    default:
        color = RGB(180, 80, 230);
        break;
    }


    fruitColorIndex++;


    HBRUSH brush =
        CreateSolidBrush(color);

    HBRUSH oldBrush =
        (HBRUSH)SelectObject(
            hdc,
            brush
        );


    Ellipse(
        hdc,
        x - r,
        y - r,
        x + r,
        y + r
    );


    SelectObject(
        hdc,
        oldBrush
    );

    DeleteObject(brush);


    // Cuống

    HPEN pen =
        CreatePen(
            PS_SOLID,
            3,
            RGB(80, 50, 20)
        );

    HPEN oldPen =
        (HPEN)SelectObject(
            hdc,
            pen
        );


    MoveToEx(
        hdc,
        x,
        y - r,
        NULL
    );

    LineTo(
        hdc,
        x + 3,
        y - r - 10
    );


    SelectObject(
        hdc,
        oldPen
    );

    DeleteObject(pen);


    // Chiếc lá

    HBRUSH leaf =
        CreateSolidBrush(
            RGB(40, 180, 70)
        );

    oldBrush =
        (HBRUSH)SelectObject(
            hdc,
            leaf
        );


    Ellipse(
        hdc,
        x + 2,
        y - r - 12,
        x + 16,
        y - r - 4
    );


    SelectObject(
        hdc,
        oldBrush
    );

    DeleteObject(leaf);
}


// ============================================================
// VẼ BOM
// ============================================================

void DrawBomb(
    HDC hdc,
    const FallingObject& object
)
{
    int x = (int)object.x;
    int y = (int)object.y;

    int r = object.radius;


    // Quả bom

    HBRUSH blackBrush =
        CreateSolidBrush(
            RGB(35, 35, 45)
        );

    HBRUSH oldBrush =
        (HBRUSH)SelectObject(
            hdc,
            blackBrush
        );


    Ellipse(
        hdc,
        x - r,
        y - r,
        x + r,
        y + r
    );


    SelectObject(
        hdc,
        oldBrush
    );

    DeleteObject(blackBrush);


    // Ngòi bom

    HPEN pen =
        CreatePen(
            PS_SOLID,
            3,
            RGB(60, 40, 20)
        );

    HPEN oldPen =
        (HPEN)SelectObject(
            hdc,
            pen
        );


    MoveToEx(
        hdc,
        x,
        y - r,
        NULL
    );

    LineTo(
        hdc,
        x + 5,
        y - r - 12
    );


    SelectObject(
        hdc,
        oldPen
    );

    DeleteObject(pen);


    // Lửa

    HBRUSH fireBrush =
        CreateSolidBrush(
            RGB(255, 180, 30)
        );

    oldBrush =
        (HBRUSH)SelectObject(
            hdc,
            fireBrush
        );


    Ellipse(
        hdc,
        x + 1,
        y - r - 17,
        x + 10,
        y - r - 7
    );


    SelectObject(
        hdc,
        oldBrush
    );

    DeleteObject(fireBrush);


    // Chữ X

    DrawTextCenter(
        hdc,
        x,
        y + 1,
        "X",
        16,
        RGB(255, 80, 80)
    );
}


// ============================================================
// VẼ GIỎ
// ============================================================

void DrawBasket(HDC hdc)
{
    int width = 130;
    int height = 45;

    int left =
        basketX - width / 2;

    int top =
        basketY;


    // Thân giỏ

    HBRUSH brush =
        CreateSolidBrush(
            RGB(150, 90, 40)
        );

    HBRUSH oldBrush =
        (HBRUSH)SelectObject(
            hdc,
            brush
        );


    RECT rect;

    rect.left = left;
    rect.top = top;
    rect.right = left + width;
    rect.bottom = top + height;


    FillRect(
        hdc,
        &rect,
        brush
    );


    SelectObject(
        hdc,
        oldBrush
    );

    DeleteObject(brush);


    // Viền

    HPEN pen =
        CreatePen(
            PS_SOLID,
            4,
            RGB(90, 50, 20)
        );

    HPEN oldPen =
        (HPEN)SelectObject(
            hdc,
            pen
        );

    HBRUSH oldBrush2 =
        (HBRUSH)SelectObject(
            hdc,
            GetStockObject(NULL_BRUSH)
        );


    Rectangle(
        hdc,
        left,
        top,
        left + width,
        top + height
    );


    SelectObject(
        hdc,
        oldPen
    );

    SelectObject(
        hdc,
        oldBrush2
    );

    DeleteObject(pen);


    // Quai giỏ

    HPEN handlePen =
        CreatePen(
            PS_SOLID,
            5,
            RGB(100, 60, 25)
        );

    oldPen =
        (HPEN)SelectObject(
            hdc,
            handlePen
        );


    Arc(
        hdc,
        left + 20,
        top - 45,
        left + width - 20,
        top + 30,
        left + width - 20,
        top + 20,
        left + 20,
        top + 20
    );


    SelectObject(
        hdc,
        oldPen
    );

    DeleteObject(handlePen);
}


// ============================================================
// VẼ HUD
// ============================================================

void DrawHUD(HDC hdc)
{
    // Điểm

    DrawTextLeft(
        hdc,
        20,
        15,
        "DIEM: " + to_string(score),
        22,
        RGB(255, 255, 255)
    );


    // Mạng

    DrawTextLeft(
        hdc,
        20,
        45,
        "MANG: " + to_string(lives),
        20,
        RGB(255, 220, 220)
    );


    // Thời gian

    DrawTextCenter(
        hdc,
        GAME_WIDTH / 2,
        30,
        "THOI GIAN: " +
        to_string(gameTime),
        22,
        RGB(255, 255, 255)
    );


    // Hướng dẫn

    DrawTextLeft(
        hdc,
        600,
        20,
        "ESC: MENU",
        15,
        RGB(255, 255, 255)
    );
}


// ============================================================
// KIỂM TRA VA CHẠM
// ============================================================

bool CheckCollision(
    const FallingObject& object
)
{
    float dx =
        object.x - basketX;

    float dy =
        object.y -
        (basketY + 15);


    float distance =
        sqrt(
            dx * dx +
            dy * dy
        );


    return distance <
        object.radius + 55;
}


// ============================================================
// CẬP NHẬT GAME
// ============================================================

void UpdateGame(HWND hwnd)
{
    DWORD now =
        GetTickCount();


    // ========================================================
    // ĐẾM THỜI GIAN
    // ========================================================

    if (
        now - lastSecondTime
        >= 1000
        )
    {
        lastSecondTime = now;

        gameTime--;

        if (gameTime <= 0)
        {
            gameTime = 0;

            gameState = GAME_OVER;

            PlaySoundEffect(
                300,
                200
            );
        }
    }


    // Nếu game kết thúc
    if (gameState != PLAYING)
    {
        InvalidateRect(
            hwnd,
            NULL,
            FALSE
        );

        return;
    }


    // ========================================================
    // TẠO VẬT THỂ MỚI
    // ========================================================

    if (
        now - lastSpawnTime
        >= 550
        )
    {
        lastSpawnTime = now;

        SpawnObject();
    }


    // ========================================================
    // DI CHUYỂN
    // ========================================================

    for (
        auto& object : objects
        )
    {
        object.y +=
            object.speed;
    }


    // ========================================================
    // KIỂM TRA VA CHẠM
    // ========================================================

    for (
        int i = (int)objects.size() - 1;
        i >= 0;
        i--
        )
    {
        FallingObject& object =
            objects[i];


        // Bắt được vật thể

        if (
            CheckCollision(object)
            )
        {
            if (
                object.type ==
                FRUIT
                )
            {
                score += 10;

                PlaySoundEffect(
                    1000,
                    40
                );
            }
            else
            {
                lives--;

                PlaySoundEffect(
                    250,
                    120
                );


                if (lives <= 0)
                {
                    lives = 0;

                    gameState =
                        GAME_OVER;
                }
            }


            objects.erase(
                objects.begin() + i
            );

            continue;
        }


        // Rơi khỏi màn hình

        if (
            object.y >
            GAME_HEIGHT + 50
            )
        {
            objects.erase(
                objects.begin() + i
            );
        }
    }


    InvalidateRect(
        hwnd,
        NULL,
        FALSE
    );
}


// ============================================================
// VẼ MÀN HÌNH CHƠI
// ============================================================

void DrawGame(HDC hdc)
{
    DrawBackground(hdc);


    // Vẽ vật thể

    for (
        const auto& object :
        objects
        )
    {
        if (
            object.type ==
            FRUIT
            )
        {
            DrawFruit(
                hdc,
                object
            );
        }
        else
        {
            DrawBomb(
                hdc,
                object
            );
        }
    }


    // Vẽ giỏ

    DrawBasket(hdc);


    // HUD

    DrawHUD(hdc);
}


// ============================================================
// VẼ CÀI ĐẶT
// ============================================================

void DrawSettings(HDC hdc)
{
    DrawBackground(hdc);


    DrawTextCenter(
        hdc,
        GAME_WIDTH / 2,
        100,
        "CAI DAT",
        42,
        RGB(255, 255, 255)
    );


    string soundText;

    if (soundEnabled)
    {
        soundText =
            "AM THANH: BAT";
    }
    else
    {
        soundText =
            "AM THANH: TAT";
    }


    Button soundButton =
    {
        BTN_SOUND,
        280,
        220,
        240,
        60,
        soundText,
        false
    };


    Button backButton =
    {
        BTN_BACK,
        280,
        310,
        240,
        60,
        "QUAY LAI",
        false
    };


    // Hover

    soundButton.hover =
        IsInside(
            mouseX,
            mouseY,
            soundButton
        );


    backButton.hover =
        IsInside(
            mouseX,
            mouseY,
            backButton
        );


    DrawButton(
        hdc,
        soundButton
    );


    DrawButton(
        hdc,
        backButton
    );


    DrawTextCenter(
        hdc,
        GAME_WIDTH / 2,
        430,
        "ESC: QUAY LAI",
        18,
        RGB(255, 255, 255)
    );
}


// ============================================================
// VẼ GAME OVER
// ============================================================

void DrawGameOver(HDC hdc)
{
    DrawBackground(hdc);


    DrawTextCenter(
        hdc,
        GAME_WIDTH / 2,
        120,
        "GAME OVER",
        50,
        RGB(255, 80, 80)
    );


    DrawTextCenter(
        hdc,
        GAME_WIDTH / 2,
        190,
        "DIEM CUA BAN",
        22,
        RGB(255, 255, 255)
    );


    DrawTextCenter(
        hdc,
        GAME_WIDTH / 2,
        235,
        to_string(score),
        45,
        RGB(255, 230, 80)
    );


    Button replay =
    {
        BTN_REPLAY,
        280,
        310,
        240,
        60,
        "CHOI LAI",
        false
    };


    Button menu =
    {
        BTN_MENU,
        280,
        390,
        240,
        60,
        "VE MENU",
        false
    };


    replay.hover =
        IsInside(
            mouseX,
            mouseY,
            replay
        );


    menu.hover =
        IsInside(
            mouseX,
            mouseY,
            menu
        );


    DrawButton(
        hdc,
        replay
    );


    DrawButton(
        hdc,
        menu
    );
}


// ============================================================
// VẼ TOÀN BỘ
// ============================================================

void DrawAll(HDC hdc)
{
    switch (gameState)
    {
    case MENU:

        DrawMenu(hdc);

        break;


    case PLAYING:

        DrawGame(hdc);

        break;


    case SETTINGS:

        DrawSettings(hdc);

        break;


    case GAME_OVER:

        DrawGameOver(hdc);

        break;
    }
}


// ============================================================
// XỬ LÝ CLICK
// ============================================================

void HandleClick(
    HWND hwnd,
    int x,
    int y
)
{
    // ========================================================
    // MENU
    // ========================================================

    if (gameState == MENU)
    {
        for (
            const auto& button :
            buttons
            )
        {
            if (
                IsInside(
                    x,
                    y,
                    button
                )
                )
            {
                switch (button.id)
                {
                case BTN_PLAY:

                    StartGame();

                    break;


                case BTN_SETTINGS:

                    gameState =
                        SETTINGS;

                    break;


                case BTN_EXIT:

                    PostQuitMessage(0);

                    break;
                }

                return;
            }
        }
    }


    // ========================================================
    // SETTINGS
    // ========================================================

    else if (
        gameState ==
        SETTINGS
        )
    {
        Button soundButton =
        {
            BTN_SOUND,
            280,
            220,
            240,
            60,
            "",
            false
        };


        Button backButton =
        {
            BTN_BACK,
            280,
            310,
            240,
            60,
            "",
            false
        };


        if (
            IsInside(
                x,
                y,
                soundButton
            )
            )
        {
            soundEnabled =
                !soundEnabled;

            PlaySoundEffect(
                700,
                70
            );

            InvalidateRect(
                hwnd,
                NULL,
                FALSE
            );
        }


        if (
            IsInside(
                x,
                y,
                backButton
            )
            )
        {
            gameState = MENU;

            InvalidateRect(
                hwnd,
                NULL,
                FALSE
            );
        }
    }


    // ========================================================
    // GAME OVER
    // ========================================================

    else if (
        gameState ==
        GAME_OVER
        )
    {
        Button replay =
        {
            BTN_REPLAY,
            280,
            310,
            240,
            60,
            "",
            false
        };


        Button menu =
        {
            BTN_MENU,
            280,
            390,
            240,
            60,
            "",
            false
        };


        if (
            IsInside(
                x,
                y,
                replay
            )
            )
        {
            StartGame();
        }


        if (
            IsInside(
                x,
                y,
                menu
            )
            )
        {
            gameState = MENU;

            objects.clear();

            InvalidateRect(
                hwnd,
                NULL,
                FALSE
            );
        }
    }
}


// ============================================================
// CẬP NHẬT HOVER
// ============================================================

void UpdateHover()
{
    if (gameState == MENU)
    {
        for (auto& button : buttons)
        {
            button.hover =
                IsInside(
                    mouseX,
                    mouseY,
                    button
                );
        }
    }
}


// ============================================================
// WINDOW PROCEDURE
// ============================================================

LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (message)
    {
        // ========================================================
        // TẠO CỬA SỔ
        // ========================================================

    case WM_CREATE:

        SetTimer(
            hwnd,
            TIMER_GAME,
            30,
            NULL
        );

        return 0;


        // ========================================================
        // TIMER GAME
        // ========================================================

    case WM_TIMER:

        if (
            wParam ==
            TIMER_GAME
            )
        {
            UpdateGame(hwnd);
        }

        return 0;


        // ========================================================
        // VẼ
        // ========================================================

    case WM_PAINT:
    {
        PAINTSTRUCT ps;

        HDC hdc =
            BeginPaint(
                hwnd,
                &ps
            );


        // Double buffering
        HDC memDC =
            CreateCompatibleDC(hdc);

        HBITMAP memBitmap =
            CreateCompatibleBitmap(
                hdc,
                GAME_WIDTH,
                GAME_HEIGHT
            );


        HBITMAP oldBitmap =
            (HBITMAP)SelectObject(
                memDC,
                memBitmap
            );


        DrawAll(memDC);


        BitBlt(
            hdc,
            0,
            0,
            GAME_WIDTH,
            GAME_HEIGHT,
            memDC,
            0,
            0,
            SRCCOPY
        );


        SelectObject(
            memDC,
            oldBitmap
        );


        DeleteObject(
            memBitmap
        );


        DeleteDC(
            memDC
        );


        EndPaint(
            hwnd,
            &ps
        );


        return 0;
    }


    // ========================================================
    // DI CHUYỂN CHUỘT
    // ========================================================

    case WM_MOUSEMOVE:
    {
        mouseX =
            LOWORD(lParam);

        mouseY =
            HIWORD(lParam);


        // Giỏ đi theo chuột

        if (
            gameState ==
            PLAYING
            )
        {
            basketX =
                mouseX;


            int halfWidth = 65;


            if (
                basketX <
                halfWidth
                )
            {
                basketX =
                    halfWidth;
            }


            if (
                basketX >
                GAME_WIDTH -
                halfWidth
                )
            {
                basketX =
                    GAME_WIDTH -
                    halfWidth;
            }
        }


        UpdateHover();


        InvalidateRect(
            hwnd,
            NULL,
            FALSE
        );


        return 0;
    }


    // ========================================================
    // CLICK CHUỘT
    // ========================================================

    case WM_LBUTTONDOWN:
    {
        int x =
            LOWORD(lParam);

        int y =
            HIWORD(lParam);


        HandleClick(
            hwnd,
            x,
            y
        );


        return 0;
    }


    // ========================================================
    // BÀN PHÍM
    // ========================================================

    case WM_KEYDOWN:
    {
        // ESC

        if (
            wParam ==
            VK_ESCAPE
            )
        {
            if (
                gameState ==
                PLAYING
                )
            {
                gameState =
                    MENU;

                objects.clear();
            }
            else if (
                gameState ==
                SETTINGS
                )
            {
                gameState =
                    MENU;
            }
            else if (
                gameState ==
                GAME_OVER
                )
            {
                gameState =
                    MENU;
            }


            InvalidateRect(
                hwnd,
                NULL,
                FALSE
            );
        }


        return 0;
    }


    // ========================================================
    // ĐÓNG CỬA SỔ
    // ========================================================

    case WM_CLOSE:

        DestroyWindow(hwnd);

        return 0;


        // ========================================================
        // HỦY CỬA SỔ
        // ========================================================

    case WM_DESTROY:

        KillTimer(
            hwnd,
            TIMER_GAME
        );

        PostQuitMessage(0);

        return 0;
    }


    return DefWindowProc(
        hwnd,
        message,
        wParam,
        lParam
    );
}


// ============================================================
// HÀM CHÍNH WINDOWS
// ============================================================

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
    // Tránh warning
    (void)hPrevInstance;
    (void)lpCmdLine;


    // Random
    srand(
        (unsigned int)
        time(NULL)
    );


    // Tạo nút menu
    CreateMenuButtons();


    // ========================================================
    // ĐĂNG KÝ WINDOW CLASS
    // ========================================================

    WNDCLASSA wc = {};

    wc.style =
        CS_HREDRAW |
        CS_VREDRAW;

    wc.lpfnWndProc =
        WindowProc;

    wc.hInstance =
        hInstance;

    wc.hCursor =
        LoadCursor(
            NULL,
            IDC_ARROW
        );

    wc.hIcon =
        LoadIcon(
            NULL,
            IDI_APPLICATION
        );

    wc.hbrBackground =
        (HBRUSH)
        GetStockObject(
            WHITE_BRUSH
        );

    wc.lpszClassName =
        "FruitCatchGame";


    if (
        !RegisterClassA(&wc)
        )
    {
        MessageBoxA(
            NULL,
            "Khong the dang ky cua so!",
            "LOI",
            MB_OK |
            MB_ICONERROR
        );

        return 0;
    }


    // ========================================================
    // KÍCH THƯỚC CỬA SỔ
    // ========================================================

    RECT rect =
    {
        0,
        0,
        GAME_WIDTH,
        GAME_HEIGHT
    };


    AdjustWindowRect(
        &rect,
        WS_OVERLAPPED |
        WS_CAPTION |
        WS_SYSMENU |
        WS_MINIMIZEBOX,
        FALSE
    );


    int windowWidth =
        rect.right -
        rect.left;


    int windowHeight =
        rect.bottom -
        rect.top;


    // ========================================================
    // TẠO CỬA SỔ
    // ========================================================

    HWND hwnd =
        CreateWindowA(

            "FruitCatchGame",

            "FRUIT CATCH GAME",

            WS_OVERLAPPED |
            WS_CAPTION |
            WS_SYSMENU |
            WS_MINIMIZEBOX,

            CW_USEDEFAULT,
            CW_USEDEFAULT,

            windowWidth,
            windowHeight,

            NULL,
            NULL,

            hInstance,

            NULL
        );


    if (hwnd == NULL)
    {
        MessageBoxA(
            NULL,
            "Khong the tao cua so!",
            "LOI",
            MB_OK |
            MB_ICONERROR
        );

        return 0;
    }


    // ========================================================
    // HIỂN THỊ
    // ========================================================

    ShowWindow(
        hwnd,
        nCmdShow
    );


    UpdateWindow(hwnd);


    // ========================================================
    // VÒNG LẶP WINDOWS
    // ========================================================

    MSG msg = {};


    while (
        GetMessage(
            &msg,
            NULL,
            0,
            0
        ) > 0
        )
    {
        TranslateMessage(&msg);

        DispatchMessage(&msg);
    }


    return (int)msg.wParam;
}