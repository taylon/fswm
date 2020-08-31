#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <xcb/xcb.h>
#include <xcb/xcb_event.h>

static xcb_connection_t *conn;
static uint16_t screen_w;
static uint16_t screen_h;

void grab_mouse_buttons(xcb_window_t *window) {
  xcb_grab_button(conn, 0, *window,
                  XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                  XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, *window, XCB_NONE,
                  1 /* left  button */, XCB_MOD_MASK_1);

  xcb_grab_button(conn, 0, *window,
                  XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                  XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, *window, XCB_NONE,
                  2 /* middle  button */, XCB_MOD_MASK_1);

  xcb_grab_button(conn, 0, *window,
                  XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                  XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, *window, XCB_NONE,
                  3 /* right  button */, XCB_MOD_MASK_1);
}

// Substructure redirection is what is going to allow us to
// receive the messages we need from X to become a Window Manager
void setup_substructure_redirection(xcb_window_t *root_window) {
  unsigned int mask[1] = {XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | // destroy notify
                          XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT}; // map request

  xcb_void_cookie_t cookie = xcb_change_window_attributes_checked(
      conn, *root_window, XCB_CW_EVENT_MASK, mask);

  xcb_flush(conn);

  xcb_generic_error_t *error = xcb_request_check(conn, cookie);
  if (error != NULL) {
    xcb_disconnect(conn);
    free(error);

    printf("unable to setup substructure redirection\n");
    exit(EXIT_FAILURE);
  }
}

void setup(void) {
  conn = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(conn)) {
    xcb_disconnect(conn);

    printf("unable to initiate connection with X server\n");
    exit(EXIT_FAILURE);
  }

  xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
  xcb_window_t *root_window = &screen->root;

  screen_w = screen->width_in_pixels;
  screen_h = screen->height_in_pixels;

  grab_mouse_buttons(root_window);
  setup_substructure_redirection(root_window);
}

void map_request(xcb_generic_event_t *generic_event) {
  printf("MapRequest\n");

  xcb_map_request_event_t *event = (xcb_map_request_event_t *)generic_event;

  xcb_map_window(conn, event->window);
}

#define XCB_CONFIG_MOVE XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
#define XCB_CONFIG_RESIZE XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
#define XCB_CONFIG_MOVE_RESIZE XCB_CONFIG_RESIZE | XCB_CONFIG_MOVE

void configure_request(xcb_generic_event_t *generic_event) {
  printf("ConfigureRequest\n");

  xcb_configure_request_event_t *event =
      (xcb_configure_request_event_t *)generic_event;

  unsigned int values[4] = {0, 0, screen_w, screen_h};
  xcb_configure_window(conn, event->window, XCB_CONFIG_MOVE_RESIZE, values);
}

static void (*event_handlers[XCB_NO_OPERATION])(xcb_generic_event_t *) = {
    [XCB_CONFIGURE_REQUEST] = configure_request,
    [XCB_MAP_REQUEST] = map_request,
};

void run(void) {
  xcb_generic_event_t *event;

  for (;;) {
    if (xcb_connection_has_error(conn)) {
      exit(EXIT_FAILURE);
    }

    if ((event = xcb_wait_for_event(conn))) {
      uint8_t event_response_type = XCB_EVENT_RESPONSE_TYPE(event);

      if (event_handlers[event_response_type]) {
        event_handlers[event_response_type](event);

        xcb_flush(conn);
      } else {
        printf("Unknown event: %i\n", event_response_type);
      }
    }

    free(event);
  }
}

int main(void) {
  setup();

  printf("Running fswm!\n");
  run();
}
