#include <X11/X.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <X11/Xlib.h>

static Display *display;
static Window rootWindow;

void setup(void) {
  if (!(display = XOpenDisplay(NULL)))
    exit(EXIT_FAILURE);

  rootWindow = DefaultRootWindow(display);

  XSelectInput(display, rootWindow,
               SubstructureRedirectMask | SubstructureNotifyMask);
  XSync(display, false);
}

void grab_keys(void) {
  XGrabKey(display, XKeysymToKeycode(display, XStringToKeysym("Return")),
           Mod1Mask, rootWindow, true, GrabModeAsync, GrabModeAsync);
}

void grab_buttons(void) {
  XGrabButton(display, 1, Mod1Mask, DefaultRootWindow(display), True,
              ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
              GrabModeAsync, GrabModeAsync, None, None);

  XGrabButton(display, 3, Mod1Mask, DefaultRootWindow(display), True,
              ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
              GrabModeAsync, GrabModeAsync, None, None);
}

void key_press(XEvent *event) {
  printf("KeyPress event\n");

  if (event->xkey.keycode ==
      XKeysymToKeycode(display, XStringToKeysym("Return")))
    system("gvim &");
}

void map_request(XEvent *event) {
  printf("MapRequest event\n");

  XMapRequestEvent *map_request_event = &event->xmaprequest;

  XMapWindow(display, map_request_event->window);
}

void configure_request(XEvent *ev) {
  printf("ConfigureRequest event\n");

  XWindowChanges window_changes;
  XConfigureRequestEvent *event = &ev->xconfigurerequest;

  window_changes.x = event->x;
  window_changes.y = event->y;
  window_changes.width = event->width;
  window_changes.height = event->height;
  window_changes.border_width = event->border_width;
  window_changes.sibling = event->above;
  window_changes.stack_mode = event->detail;

  XConfigureWindow(display, event->window, event->value_mask, &window_changes);
}

static void (*event_handlers[LASTEvent])(XEvent *) = {
    [KeyPress] = key_press,
    [ConfigureRequest] = configure_request,
    [MapRequest] = map_request,
};

void run(void) {
  XEvent event;

  XSync(display, False);

  while (!XNextEvent(display, &event)) {
    printf("Event: %i\n", event.type);

    if (event_handlers[event.type])
      event_handlers[event.type](&event);
  }
}

int main(void) {
  setup();

  grab_keys();
  grab_buttons();

  printf("Running!\n");
  run();
}
