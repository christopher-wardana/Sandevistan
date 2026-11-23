#include "gui.h"
#include "raylib.h"
#include "parseJson.h"
#include "plugin_registry.h"
#include <string>

using namespace std;


void gui::run_gui()
{

    // Initialization
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Basic Sandy");

    string name;
    string printed_text;

    Rectangle textBox = { 0, 180, screenWidth, 50 };
    bool mouseOnText = false;
    
    Rectangle btnBounds = { 0, 0, 100, 50 };

    bool btnAction = false;         // Button action should be activated
    Vector2 mousePoint = { 0.0f, 0.0f };

    bool printText = false;

    
    PluginRegistry registry;
    registry.loadAll(PLUGIN_DIR);

    SetTargetFPS(30); 
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (CheckCollisionPointRec(GetMousePosition(), textBox)) mouseOnText = true;
        else mouseOnText = false;

        if (mouseOnText)
        {
            // Set the window's cursor to the I-Beam
            SetMouseCursor(MOUSE_CURSOR_IBEAM);

            // Get char pressed (unicode character) on the queue
            int key = GetCharPressed();

            // Check if more characters have been pressed on the same frame
            while (key > 0)
            {
                // NOTE: Only allow keys in range [32..125]
                if ((key >= 32) && (key <= 125))
                {
                    name += (char)key;  
                }

                key = GetCharPressed();  // Check next character in the queue
            }

            if (IsKeyPressed(KEY_BACKSPACE) && !name.empty())
            {
                name.pop_back();   
            }
        }
        else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

        // Update Mouse
        //----------------------------------------------------------------------------------
        mousePoint = GetMousePosition();
        btnAction = false;

        // Check button state
         if (CheckCollisionPointRec(mousePoint, btnBounds))
        {

            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) btnAction = true;
        }
        
        if (btnAction)
        {
            printText = true;

            if (registry.dispatch(name)) printed_text = "Done: " + name;
            else printed_text = "No plugin understood: " + name;

        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawRectangleRec(textBox, LIGHTGRAY);
            if (mouseOnText) DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, RED);
            else DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);
            
            DrawText(name.c_str(), (int)textBox.x + 5, (int)textBox.y + 8, 40, MAROON);

            DrawRectangleRec(btnBounds, LIGHTGRAY);

            if(printText){
                DrawText(printed_text.c_str(), 20, 400, 40, MAROON);
            } 

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();        // Close window and OpenGL context

}