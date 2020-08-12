#ifndef __SENSOR_ADJUSTMENT_PAGE_H__
#define __SENSOR_ADJUSTMENT_PAGE_H__

// uncomment this for 5-panel (PIU) mode
// #define __5_PANEL__ 
#define NUM_PLAYERS 1

#include <EmbAJAX.h>

#define BUFLEN 30


// display for the current polling rate
EmbAJAXMutableSpan pollratedisplay("pollratedisplay");

// displays for the current input values
EmbAJAXMutableSpan p1a1display("p1a1display");
EmbAJAXMutableSpan p1a2display("p1a2display");
EmbAJAXMutableSpan p1a3display("p1a3display");
EmbAJAXMutableSpan p1a4display("p1a4display");
EmbAJAXMutableSpan p1a5display("p1a5display");
EmbAJAXMutableSpan p2a1display("p2a1display");
EmbAJAXMutableSpan p2a2display("p2a2display");
EmbAJAXMutableSpan p2a3display("p2a3display");
EmbAJAXMutableSpan p2a4display("p2a4display");
EmbAJAXMutableSpan p2a5display("p2a5display");

// text inputs to adjust the threshold values
EmbAJAXTextInput<BUFLEN> p1a1threshold("p1a1threshold");
EmbAJAXTextInput<BUFLEN> p1a2threshold("p1a2threshold");
EmbAJAXTextInput<BUFLEN> p1a3threshold("p1a3threshold");
EmbAJAXTextInput<BUFLEN> p1a4threshold("p1a4threshold");
EmbAJAXTextInput<BUFLEN> p1a5threshold("p1a5threshold");
EmbAJAXTextInput<BUFLEN> p2a1threshold("p2a1threshold");
EmbAJAXTextInput<BUFLEN> p2a2threshold("p2a2threshold");
EmbAJAXTextInput<BUFLEN> p2a3threshold("p2a3threshold");
EmbAJAXTextInput<BUFLEN> p2a4threshold("p2a4threshold");
EmbAJAXTextInput<BUFLEN> p2a5threshold("p2a5threshold");

#ifdef __5_PANEL__
  #define PAGE_TITLE "5-Panel Pad Configuration"
  #define NUM_INPUTS 5
  #define ARROW_1 "Panel 1"
  #define ARROW_2 "Panel 2"
  #define ARROW_3 "Panel 3"
  #define ARROW_4 "Panel 4"
  #define ARROW_5 "Panel 5"
#else
  #define PAGE_TITLE "4-Panel Pad Configuration"
  #define NUM_INPUTS 4
  #define ARROW_1 "Panel 1"
  #define ARROW_2 "Panel 2"
  #define ARROW_3 "Panel 3"
  #define ARROW_4 "Panel 4"
#endif

// construct the page itself
MAKE_EmbAJAXPage(page, PAGE_TITLE, "",
    new EmbAJAXStatic(""
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
            "<meta name=\"viewport\" content\"width=device-width\">"
            "<style>"
                "html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"
                "table, th, td { border: 1px solid black; }"
                "th, td { padding: 5px }"
                "table { margin-left: auto; margin-right: auto }"
            "</style>"
        "</head>"
        "<body>"
           "<h1>"
           PAGE_TITLE
           "</h1>"
            "<h2>Arduino FSR poll rate: "),
    &pollratedisplay,
    new EmbAJAXStatic(""
    "<b> Hz</b></h2>"
    "<h2>Current input values:</h2>"
    "<table>"
    "<tbody>"
    "<tr><td><b>"
    ARROW_1              
    ": </b>"),
    &p1a1display,
    new EmbAJAXStatic("</td>"),
#if NUM_PLAYERS == 2
    new EmbAJAXStatic("<td><b>"
    ARROW_1
    ": </b>"),
    &p2a1display,
    new EmbAJAXStatic("</td>"),
#endif
    new EmbAJAXStatic("</tr><tr><td><b>"
    ARROW_2
    ": </b>"),
    &p1a2display,
    new EmbAJAXStatic("</td>"),
#if NUM_PLAYERS == 2
    new EmbAJAXStatic("<td><b>"
    ARROW_2
    ": </b>"),
    &p2a2display,
    new EmbAJAXStatic("</td>"),
#endif
    new EmbAJAXStatic("</tr><tr><td><b>"
    ARROW_3
    ": </b>"),
    &p1a3display,
    new EmbAJAXStatic("</td>"),
#if NUM_PLAYERS == 2
    new EmbAJAXStatic("<td><b>"
    ARROW_3
    ": </b>"),
    &p2a3display,
    new EmbAJAXStatic("</td>"),
#endif
    new EmbAJAXStatic("</tr><tr><td><b>"
    ARROW_4
    ": </b>"),
    &p1a4display,
    new EmbAJAXStatic("</td>"),
#if NUM_PLAYERS == 2
    new EmbAJAXStatic("<td><b>"
    ARROW_4
    ": </b>"),
    &p2a4display,
    new EmbAJAXStatic("</td>"),
#endif
    new EmbAJAXStatic("</tr>"),
#ifdef __5_PANEL__
    new EmbAJAXStatic("<tr><td><b>"
    ARROW_5
    ": </b>"),
    &p1a5display,
    new EmbAJAXStatic("</td>"),
#if NUM_PLAYERS == 2
    new EmbAJAXStatic("<td><b>"
    ARROW_5
    ": </b>"),
    &p2a5display,
    new EmbAJAXStatic("</td>"),
#endif
    new EmbAJAXStatic("</tr>"),
#endif
    new EmbAJAXStatic("</tbody>"
        "</table>"
        "<h2>Adjust sensitivities:</h2>"
        "<table>"
            "<tbody>"
                "<tr><td><b>"
                ARROW_1
                ": </b>"),
    &p1a1threshold,
    new EmbAJAXStatic("</td>"),
#if NUM_PLAYERS == 2
    new EmbAJAXStatic("<td><b>"
    ARROW_1
    ": </b>"),
    &p2a1threshold,
    new EmbAJAXStatic("</td>"),
#endif
    new EmbAJAXStatic("</tr><tr><td><b>"
    ARROW_2
    ": </b>"),
    &p1a2threshold,
    new EmbAJAXStatic("</td>"),
#if NUM_PLAYERS == 2
    new EmbAJAXStatic("<td><b>"
    ARROW_2
    ": </b>"),
    &p2a2threshold,
    new EmbAJAXStatic("</td>"),
#endif
    new EmbAJAXStatic("</tr><tr><td><b>"
    ARROW_3
    ": </b>"),
    &p1a3threshold,
    new EmbAJAXStatic("</td>"),
#if NUM_PLAYERS == 2
    new EmbAJAXStatic("<td><b>"
    ARROW_3
    ": </b>"),
    &p2a3threshold,
    new EmbAJAXStatic("</td>"),
#endif
    new EmbAJAXStatic("</tr><tr><td><b>"
    ARROW_4
    ": </b>"),
    &p1a4threshold,
    new EmbAJAXStatic("</td>"),
#if NUM_PLAYERS == 2
    new EmbAJAXStatic("<td><b>"
    ARROW_4
    ": </b>"),
    &p2a4threshold,
    new EmbAJAXStatic("</td>"),
#endif
    new EmbAJAXStatic("</tr>"),
#ifdef __5_PANEL__
    new EmbAJAXStatic("<tr><td><b>"
    ARROW_5
    ": </b>"),
    &p1a5threshold,
    new EmbAJAXStatic("</td>"),
#if NUM_PLAYERS == 2
    new EmbAJAXStatic("<td><b>"
    ARROW_5
    ": </b>"),
    &p2a5threshold,
    new EmbAJAXStatic("</td>"),
#endif
    new EmbAJAXStatic("</tr>"),
#endif
    new EmbAJAXStatic("</tbody>"
        "</table>"
        "</body>"
        "</html>")
)

#endif
