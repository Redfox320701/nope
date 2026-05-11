#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TOTAL_PETALS 30
#define CENTER_X 64
#define CENTER_Y 36
#define PETAL_RADIUS 18
#define PETAL_OFFSET 10
#define FLOWER_CORE_RADIUS 7

typedef enum {
    StateIntro,
    StatePlucking,
    StateResult,
} AppState;

typedef struct {
    AppState state;
    int petals_remaining;
    bool loves_me;        // current petal text
    bool final_result;    // randomized ending
    bool result_known;    // has the final result been pre-determined
} LovesMeApp;

// Draw a small filled circle (petal dot) at given position
static void draw_petal(Canvas* canvas, int cx, int cy, float angle_deg, bool filled) {
    float angle = angle_deg * 3.14159f / 180.0f;
    int px = (int)(cx + (PETAL_OFFSET + 4) * cosf(angle));
    int py = (int)(cy + (PETAL_OFFSET + 4) * sinf(angle));
    if(filled) {
        canvas_draw_disc(canvas, px, py, 4);
    } else {
        canvas_draw_circle(canvas, px, py, 4);
    }
}

static void draw_flower(Canvas* canvas, LovesMeApp* app) {
    // Draw petals
    for(int i = 0; i < app->petals_remaining; i++) {
        float angle = (360.0f / TOTAL_PETALS) * i;
        draw_petal(canvas, CENTER_X, CENTER_Y, angle, true);
    }
    // Draw flower center
    canvas_draw_disc(canvas, CENTER_X, CENTER_Y, FLOWER_CORE_RADIUS);
    // Draw small white dot in center for detail
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_disc(canvas, CENTER_X, CENTER_Y, FLOWER_CORE_RADIUS - 2);
    canvas_set_color(canvas, ColorBlack);
}

static void render_callback(Canvas* canvas, void* ctx) {
    LovesMeApp* app = ctx;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);

    if(app->state == StateIntro) {
        canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignTop, "Loves Me?");
        draw_flower(canvas, app);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 56, AlignCenter, AlignTop, "Press OK to pluck");
        return;
    }

    if(app->state == StatePlucking) {
        // Show current loves me / loves me not
        const char* phrase = app->loves_me ? "Loves me!" : "Loves me not...";
        canvas_draw_str_aligned(canvas, 64, 0, AlignCenter, AlignTop, phrase);
        draw_flower(canvas, app);
        // Petals remaining counter
        char buf[24];
        snprintf(buf, sizeof(buf), "%d left", app->petals_remaining);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 56, AlignCenter, AlignTop, buf);
        return;
    }

    if(app->state == StateResult) {
        canvas_set_font(canvas, FontPrimary);
        if(app->final_result) {
            canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignTop, "LOVES ME!");
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(canvas, 64, 28, AlignCenter, AlignTop, "The flower says yes~");
        } else {
            canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignTop, "Loves me not...");
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(canvas, 64, 28, AlignCenter, AlignTop, "Maybe next time :(");
        }
        canvas_draw_str_aligned(canvas, 64, 44, AlignCenter, AlignTop, "* * *");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 54, AlignCenter, AlignTop, "Hold Back to restart");
        return;
    }
}

static void input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* queue = ctx;
    furi_message_queue_put(queue, input_event, FuriWaitForever);
}

int32_t loves_me_app(void* p) {
    UNUSED(p);

    LovesMeApp* app = malloc(sizeof(LovesMeApp));
    app->state = StateIntro;
    app->petals_remaining = TOTAL_PETALS;
    app->loves_me = true;
    app->result_known = false;

    // Seed random with tick count
    srand((unsigned int)furi_get_tick());

    // Pre-determine final result randomly
    // Compute: if petals_remaining is even when we reach 0,
    // last petal is "loves me not" (starts at "loves me" for petal 30).
    // We override by adjusting — just pick randomly and remember it.
    app->final_result = (rand() % 2) == 0;
    app->result_known = true;

    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    ViewPort* viewport = view_port_alloc();
    view_port_draw_callback_set(viewport, render_callback, app);
    view_port_input_callback_set(viewport, input_callback, queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, viewport, GuiLayerFullscreen);

    InputEvent event;
    bool running = true;

    while(running) {
        if(furi_message_queue_get(queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypeShort || event.type == InputTypeLong) {
                if(app->state == StateIntro) {
                    if(event.key == InputKeyOk || event.key == InputKeyRight) {
                        app->state = StatePlucking;
                        // First pluck
                        app->petals_remaining--;
                        app->loves_me = true; // first petal = "loves me"
                    } else if(event.key == InputKeyBack) {
                        running = false;
                    }
                } else if(app->state == StatePlucking) {
                    if(event.key == InputKeyOk || event.key == InputKeyRight) {
                        if(app->petals_remaining > 0) {
                            app->petals_remaining--;
                            // Alternate phrase
                            app->loves_me = !app->loves_me;

                            // When last petal reached, decide result
                            if(app->petals_remaining == 0) {
                                app->state = StateResult;
                                // final_result was pre-randomized
                            }
                        }
                    } else if(event.key == InputKeyBack) {
                        running = false;
                    }
                } else if(app->state == StateResult) {
                    if(event.key == InputKeyBack && event.type == InputTypeLong) {
                        // Restart
                        app->state = StateIntro;
                        app->petals_remaining = TOTAL_PETALS;
                        app->loves_me = true;
                        app->final_result = (rand() % 2) == 0;
                    } else if(event.key == InputKeyBack) {
                        running = false;
                    }
                }
            }
        }
        view_port_update(viewport);
    }

    gui_remove_view_port(gui, viewport);
    view_port_free(viewport);
    furi_record_close(RECORD_GUI);
    furi_message_queue_free(queue);
    free(app);

    return 0;
}
