#include "app.h"

int main(void) {
    App app = {0};
    AppInit(&app);

    while (app.is_running) {
        AppUpdate(&app);
    }

    AppQuit(&app);
    return 0;
}
