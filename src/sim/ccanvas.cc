//==========================================================================
//   CCANVAS.CC  -  part of
//
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2015 Andras Varga
  Copyright (C) 2006-2015 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <map>
#include <algorithm>
#include "common/stringutil.h"
#include "common/colorutil.h"
#include "common/opp_ctype.h"
#include "omnetpp/ccanvas.h"
#include "omnetpp/cproperty.h"
#include "omnetpp/cproperties.h"
#include "omnetpp/cstringtokenizer.h"

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

using namespace omnetpp::common;

namespace omnetpp {

using namespace canvas_stream_ops;

Register_Class(cGroupFigure);
//Register_Class(cPanelFigure);
Register_Class(cLineFigure);
Register_Class(cArcFigure);
Register_Class(cPolylineFigure);
Register_Class(cRectangleFigure);
Register_Class(cOvalFigure);
Register_Class(cRingFigure);
Register_Class(cPieSliceFigure);
Register_Class(cPolygonFigure);
Register_Class(cPathFigure);
Register_Class(cTextFigure);
Register_Class(cLabelFigure);
Register_Class(cImageFigure);
Register_Class(cIconFigure);
Register_Class(cPixmapFigure);

const cFigure::Color cFigure::BLACK(0, 0, 0);
const cFigure::Color cFigure::WHITE(255, 255, 255);
const cFigure::Color cFigure::GREY(128, 128, 128);
const cFigure::Color cFigure::RED(255, 0, 0);
const cFigure::Color cFigure::GREEN(0, 255, 0);
const cFigure::Color cFigure::BLUE(0, 0, 255);
const cFigure::Color cFigure::YELLOW(255, 255, 0);
const cFigure::Color cFigure::CYAN(0, 255, 255);
const cFigure::Color cFigure::MAGENTA(255, 0, 255);

const cFigure::Color cFigure::GOOD_DARK_COLORS[] = {
    cFigure::parseColor("darkBlue"),
    cFigure::parseColor("red2"),
    cFigure::parseColor("darkGreen"),
    cFigure::parseColor("orange"),
    cFigure::parseColor("#008000"),
    cFigure::parseColor("darkGrey"),
    cFigure::parseColor("#a00000"),
    cFigure::parseColor("#008080"),
    cFigure::parseColor("cyan"),
    cFigure::parseColor("#808000"),
    cFigure::parseColor("#8080ff"),
    cFigure::parseColor("yellow"),
    cFigure::parseColor("black"),
    cFigure::parseColor("purple"),
};

const cFigure::Color cFigure::GOOD_LIGHT_COLORS[] = {
    cFigure::parseColor("lightCyan"),
    cFigure::parseColor("lightCoral"),
    cFigure::parseColor("lightBlue"),
    cFigure::parseColor("lightGreen"),
    cFigure::parseColor("lightYellow"),
    cFigure::parseColor("plum1"),
    cFigure::parseColor("lightSalmon"),
    cFigure::parseColor("lightPink"),
    cFigure::parseColor("lightGrey"),
    cFigure::parseColor("mediumPurple"),
};

static const char *PKEY_TYPE = "type";
static const char *PKEY_VISIBLE = "visible";
static const char *PKEY_TAGS = "tags";
static const char *PKEY_TRANSFORM = "transform";
static const char *PKEY_CHILDZ = "childZ";
static const char *PKEY_POS = "pos";
static const char *PKEY_POINTS = "points";
static const char *PKEY_ANCHOR = "anchor";
static const char *PKEY_SIZE = "size";
static const char *PKEY_INNERSIZE = "innerSize";
static const char *PKEY_CORNERRADIUS = "cornerRadius";
static const char *PKEY_STARTANGLE = "startAngle";
static const char *PKEY_ENDANGLE = "endAngle";
static const char *PKEY_PATH = "path";
static const char *PKEY_SMOOTH = "smooth";
static const char *PKEY_OFFSET = "offset";
static const char *PKEY_LINECOLOR = "lineColor";
static const char *PKEY_LINESTYLE = "lineStyle";
static const char *PKEY_LINEWIDTH = "lineWidth";
static const char *PKEY_LINEOPACITY = "lineOpacity";
static const char *PKEY_CAPSTYLE = "capStyle";
static const char *PKEY_JOINSTYLE = "joinStyle";
static const char *PKEY_ZOOMLINEWIDTH = "zoomLineWidth";
static const char *PKEY_FILLCOLOR = "fillColor";
static const char *PKEY_FILLOPACITY = "fillOpacity";
static const char *PKEY_FILLRULE = "fillRule";
static const char *PKEY_STARTARROWHEAD = "startArrowhead";
static const char *PKEY_ENDARROWHEAD = "endArrowhead";
static const char *PKEY_TEXT = "text";
static const char *PKEY_FONT = "font";
static const char *PKEY_COLOR = "color";
static const char *PKEY_OPACITY = "opacity";
static const char *PKEY_IMAGE = "image";
static const char *PKEY_RESOLUTION = "resolution";
static const char *PKEY_INTERPOLATION = "interpolation";
static const char *PKEY_TINT = "tint";

int cFigure::lastId = 0;
cStringPool cFigure::stringPool;

#define ENSURE_RANGE01(var)  {if (var < 0 || var > 1) throw cRuntimeError(this, #var " must be in the range [0,1]");}
#define ENSURE_NONNEGATIVE(var)  {if (var < 0) throw cRuntimeError(this, #var " cannot be negative");}
#define ENSURE_POSITIVE(var)  {if (var <= 0) throw cRuntimeError(this, #var " cannot be negative or zero");}

std::string cFigure::Point::str() const
{
    std::stringstream os;
    os << "(" << x << ", " << y << ")";
    return os.str();
}

cFigure::Point cFigure::Point::operator+(const Point& p) const
{
    return Point(x + p.x, y + p.y);
}

cFigure::Point cFigure::Point::operator-(const cFigure::Point& p) const
{
    return Point(x - p.x, y - p.y);
}

cFigure::Point cFigure::Point::operator*(double s) const
{
    return Point(x * s, y * s);
}

cFigure::Point cFigure::Point::operator/(double s) const
{
    return Point(x / s, y / s);
}

double cFigure::Point::operator*(const Point& p) const
{
    return x * p.x + y * p.y;
}

double cFigure::Point::getLength() const
{
    return sqrt(x * x + y * y);
}

double cFigure::Point::distanceTo(const Point& p) const
{
    double dx = p.x - x;
    double dy = p.y - y;
    return sqrt(dx * dx + dy * dy);
}

std::string cFigure::Rectangle::str() const
{
    std::stringstream os;
    os << "(" << x << ", " << y << ", w=" << width << ", h=" << height << ")";
    return os.str();
}

cFigure::Point cFigure::Rectangle::getCenter() const
{
    return Point(x + width / 2.0, y + height / 2.0);
}

cFigure::Point cFigure::Rectangle::getSize() const
{
    return Point(width, height);
}

std::string cFigure::Color::str() const
{
    std::stringstream os;
    os << "(" << (int)red << ", " << (int)green << ", " << (int)blue << ")";
    return os.str();
}

std::string cFigure::Font::str() const
{
    std::stringstream os;
    os << "(" << (typeface.empty() ? "<default>" : typeface) << ", ";
    os << (int)pointSize << "pt";
    if (style) {
        os << ",";
        if (style & cFigure::FONT_BOLD)  os << " bold";
        if (style & cFigure::FONT_ITALIC)  os << " italic";
        if (style & cFigure::FONT_UNDERLINE)  os << " underline";
    }
    os << ")";
    return os.str();
}

std::string cFigure::RGBA::str() const
{
    std::stringstream os;
    os << "(" << (int)red << ", " << (int)green << ", " << (int)blue << ", " << (int)alpha << ")";
    return os.str();
}

cFigure::Transform& cFigure::Transform::translate(double dx, double dy)
{
    t1 += dx;
    t2 += dy;
    return *this;
}

cFigure::Transform& cFigure::Transform::scale(double sx, double sy)
{
    *this = Transform(a*sx, b*sy, c*sx, d*sy, sx*t1, sy*t2);
    return *this;
}

cFigure::Transform& cFigure::Transform::scale(double sx, double sy, double cx, double cy)
{
    *this = Transform(a*sx, b*sy, c*sx, d*sy, sx*t1-cx*sx+cx, sy*t2-cy*sy+cy);
    return *this;
}

cFigure::Transform& cFigure::Transform::rotate(double phi)
{
    double cosPhi = cos(phi);
    double sinPhi = sin(phi);
    *this = Transform(
            a*cosPhi - b*sinPhi, a*sinPhi + b*cosPhi,
            c*cosPhi - d*sinPhi, c*sinPhi + d*cosPhi,
            t1*cosPhi - t2*sinPhi, t1*sinPhi + t2*cosPhi);
    return *this;
}

cFigure::Transform& cFigure::Transform::rotate(double phi, double cx, double cy)
{
    double cosPhi = cos(phi);
    double sinPhi = sin(phi);
    *this = Transform(
            a*cosPhi - b*sinPhi, a*sinPhi + b*cosPhi,
            c*cosPhi - d*sinPhi, c*sinPhi + d*cosPhi,
            -t2*sinPhi+cy*sinPhi+t1*cosPhi-cx*cosPhi+cx, t1*sinPhi-cx*sinPhi+t2*cosPhi-cy*cosPhi+cy);
    return *this;
}

cFigure::Transform& cFigure::Transform::skewx(double phi)
{
    double tanPhy = tan(phi);
    double a_ = b*tanPhy+a;
    double c_ = d*tanPhy+c;
    double t1_ = t2*tanPhy+t1;
    a = a_; c = c_; t1 = t1_;
    return *this;
}

cFigure::Transform& cFigure::Transform::skewy(double phi)
{
    double tanPhy = tan(phi);
    double b_ = a*tanPhy+b;
    double d_ = c*tanPhy+d;
    double t2_ = t1*tanPhy+t2;
    b = b_; d = d_; t2 = t2_;
    return *this;
}

cFigure::Transform& cFigure::Transform::skewx(double phi, double cy)
{
    double tanPhy = tan(phi);
    double a_ = b*tanPhy+a;
    double c_ = d*tanPhy+c;
    double t1_ = t2*tanPhy-cy*tanPhy+t1;
    a = a_; c = c_; t1 = t1_;
    return *this;
}

cFigure::Transform& cFigure::Transform::skewy(double phi, double cx)
{
    double tanPhy = tan(phi);
    double b_ = a*tanPhy+b;
    double d_ = c*tanPhy+d;
    double t2_ = t1*tanPhy-cx*tanPhy+t2;
    b = b_; d = d_; t2 = t2_;
    return *this;
}

cFigure::Transform& cFigure::Transform::multiply(const Transform& other)
{
    double a_ = a*other.a + b*other.c;
    double b_ = a*other.b + b*other.d;
    double c_ = c*other.a + d*other.c;
    double d_ = c*other.b + d*other.d;
    double t1_ = t1*other.a + t2*other.c + other.t1;
    double t2_ = t1*other.b + t2*other.d + other.t2;

    a = a_; b = b_;
    c = c_; d = d_;
    t1 = t1_; t2 = t2_;

    return *this;
}

cFigure::Transform& cFigure::Transform::leftMultiply(const Transform& other)
{
    double a_ = other.a*a + other.b*c;
    double b_ = other.a*b + other.b*d;
    double c_ = other.c*a + other.d*c;
    double d_ = other.c*b + other.d*d;
    double t1_ = other.t1*a + other.t2*c + t1;
    double t2_ = other.t1*b + other.t2*d + t2;

    a = a_; b = b_;
    c = c_; d = d_;
    t1 = t1_; t2 = t2_;

    return *this;
}


cFigure::Point cFigure::Transform::applyTo(const Point& p) const
{
    return Point(a*p.x + c*p.y + t1, b*p.x + d*p.y + t2);
}

std::string cFigure::Transform::str() const
{
    std::stringstream os;
    os << "((" << a << " " << b << ")";
    os << " (" << c << " " << d << ")";
    os << " (" << t1 << " " << t2 << "))";
    return os.str();
}

//----

cFigure::Pixmap::Pixmap() : width(0), height(0), data(nullptr)
{
}

cFigure::Pixmap::Pixmap(int width, int height) : width(width), height(height), data(nullptr)
{
    allocate(width, height);
    fill(BLACK, 0);
}

cFigure::Pixmap::Pixmap(int width, int height, const RGBA& fill_) : width(width), height(height), data(nullptr)
{
    allocate(width, height);
    fill(fill_);
}

cFigure::Pixmap::Pixmap(int width, int height, const Color& color, double opacity) : width(width), height(height), data(nullptr)
{
    allocate(width, height);
    fill(color, opacity);
}

cFigure::Pixmap::Pixmap(const Pixmap& other) : width(0), height(0), data(nullptr)
{
    *this = other;
}

cFigure::Pixmap::~Pixmap()
{
    delete[] data;
}

void cFigure::Pixmap::allocate(int newWidth, int newHeight)
{
    if (newWidth < 0 || newHeight < 0)
        throw cRuntimeError("cFigure::Pixmap: width/height cannot be negative");

    width = newWidth;
    height = newHeight;
    delete[] data;
    data = new RGBA[newWidth * newHeight];
}

void cFigure::Pixmap::resize(int newWidth, int newHeight, const RGBA& fill_)
{
    if (newWidth == width && newHeight == height)
        return; // no change
    if (newWidth < 0 || newWidth < 0)
        throw cRuntimeError("cFigure::Pixmap: width/height cannot be negative");

    RGBA *newData = new RGBA[newWidth * newHeight];

    // pixel copying (could be more efficient)
    for (int y = 0; y < newHeight; y++)
        for (int x = 0; x < newWidth; x++)
            newData[y * newWidth + x] = (x < width && y < height) ? data[y * width + x] : fill_;


    width = newWidth;
    height = newHeight;
    delete[] data;
    data = newData;
}

void cFigure::Pixmap::resize(int width, int height, const Color& color, double opacity)
{
    RGBA rgba(color.red, color.green, color.blue, alpha(opacity));
    resize(width, height, rgba);
}

void cFigure::Pixmap::fill(const RGBA& rgba)
{
    int size = width * height;
    for (int i = 0; i < size; i++)
        data[i] = rgba;
}

void cFigure::Pixmap::fill(const Color& color, double opacity)
{
    RGBA rgba(color.red, color.green, color.blue, alpha(opacity));
    fill(rgba);
}

cFigure::Pixmap& cFigure::Pixmap::operator=(const Pixmap& other)
{
    if (width != other.width || height != other.height)
        allocate(other.width, other.height);
    memcpy(data, other.data, width * height * sizeof(RGBA));
    return *this;
}

cFigure::RGBA& cFigure::Pixmap::pixel(int x, int y)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
        throw cRuntimeError("cFigure::Pixmap: x or y coordinate out of bounds");
    return data[width * y + x];
}

std::string cFigure::Pixmap::str() const
{
    std::stringstream os;
    os << "(" << width << " x " << height << ")";
    return os.str();
}

//----

cFigure::Point cFigure::parsePoint(cProperty *property, const char *key, int index)
{
    double x = opp_atof(opp_nulltoempty(property->getValue(key, index)));
    double y = opp_atof(opp_nulltoempty(property->getValue(key, index + 1)));
    return Point(x, y);
}

std::vector<cFigure::Point> cFigure::parsePoints(cProperty *property, const char *key)
{
    std::vector<Point> points;
    int n = property->getNumValues(key);
    if (n % 2 != 0)
        throw cRuntimeError("number of coordinates must be even");
    for (int i = 0; i < n; i += 2)
        points.push_back(parsePoint(property, key, i));
    return points;
}

void cFigure::parseBounds(cProperty *property, Point& p1, Point& p2)
{
    int numCoords = property->getNumValues(PKEY_POS);
    if (numCoords != 2)
        throw cRuntimeError("position: two coordinates expected");
    Point p = parsePoint(property, PKEY_POS, 0);
    Point size = parsePoint(property, PKEY_SIZE, 0);
    const char *anchorStr = property->getValue(PKEY_ANCHOR);
    Anchor anchor = opp_isblank(anchorStr) ? cFigure::ANCHOR_NW : parseAnchor(anchorStr);
    switch (anchor) {
        case cFigure::ANCHOR_CENTER:
            p1.x = p.x - size.x / 2;
            p1.y = p.y - size.y / 2;
            p2.x = p.x + size.x / 2;
            p2.y = p.y + size.y / 2;
            break;

        case cFigure::ANCHOR_N:
            p1.x = p.x - size.x / 2;
            p1.y = p.y;
            p2.x = p.x + size.x / 2;
            p2.y = p.y + size.y;
            break;

        case cFigure::ANCHOR_E:
            p1.x = p.x;
            p1.y = p.y - size.y / 2;
            p2.x = p.x + size.x;
            p2.y = p.y + size.y / 2;
            break;

        case cFigure::ANCHOR_S:
        case cFigure::ANCHOR_BASELINE_MIDDLE:
            p1.x = p.x - size.x / 2;
            p1.y = p.y - size.y;
            p2.x = p.x + size.x / 2;
            p2.y = p.y;
            break;

        case cFigure::ANCHOR_W:
            p1.x = p.x - size.x;
            p1.y = p.y - size.y / 2;
            p2.x = p.x;
            p2.y = p.y + size.y / 2;
            break;

        case cFigure::ANCHOR_NW:
            p1.x = p.x;
            p1.y = p.y;
            p2.x = p.x + size.x;
            p2.y = p.y + size.y;
            break;

        case cFigure::ANCHOR_NE:
            p1.x = p.x - size.x;
            p1.y = p.y;
            p2.x = p.x;
            p2.y = p.y + size.y;
            break;

        case cFigure::ANCHOR_SE:
        case cFigure::ANCHOR_BASELINE_END:
            p1.x = p.x - size.x;
            p1.y = p.y - size.y;
            p2.x = p.x;
            p2.y = p.y;
            break;

        case cFigure::ANCHOR_SW:
        case cFigure::ANCHOR_BASELINE_START:
            p1.x = p.x;
            p1.y = p.y - size.y;
            p2.x = p.x + size.x;
            p2.y = p.y;
            break;

        default:
            throw cRuntimeError("Unexpected anchor %d", anchor);
    }
}

cFigure::Rectangle cFigure::parseBounds(cProperty *property)
{
    Point p1, p2;
    parseBounds(property, p1, p2);
    return Rectangle(p1.x, p1.y, p2.x - p1.x, p2.y - p1.y);
}

inline bool contains(const std::string& str, const std::string& substr)
{
    return str.find(substr) != std::string::npos;
}

inline double deg2rad(double deg) { return deg * M_PI / 180; }
inline double rad2deg(double rad) { return rad / M_PI * 180; }

cFigure::Transform cFigure::parseTransform(cProperty *property, const char *key)
{
    Transform transform;
    for (int index = 0; index < property->getNumValues(key); index++) {
        const char *step = property->getValue(key, index);
        std::string operation = opp_trim(opp_substringbefore(step, "(").c_str());
        std::string rest = opp_trim(opp_substringafter(step, "(").c_str());
        bool missingRParen = !contains(rest, ")");
        std::string argsStr = opp_trim(opp_substringbefore(rest, ")").c_str());
        std::string trailingGarbage = opp_trim(opp_substringafter(rest, ")").c_str());
        if (operation == "" || missingRParen || trailingGarbage.size() != 0)
            throw cRuntimeError("Syntax error in '%s', 'operation(arg1,arg2...)' expected", step);
        std::vector<double> args = cStringTokenizer(argsStr.c_str(), ",").asDoubleVector();

        if (operation == "translate") {
            if (args.size() == 2)
                transform.translate(args[0], args[1]);
            else
                throw cRuntimeError("Wrong number of args in '%s', translate(dx,dxy) expected", step);
        }
        else if (operation == "rotate") {
            if (args.size() == 1)
                transform.rotate(deg2rad(args[0]));
            else if (args.size() == 3)
                transform.rotate(deg2rad(args[0]), args[1], args[2]);
            else
                throw cRuntimeError("Wrong number of args in '%s', rotate(deg) or rotate(deg,cx,cy) expected", step);
        }
        else if (operation == "scale") {
            if (args.size() == 1)
                transform.scale(args[0]);
            else if (args.size() == 2)
                transform.scale(args[0], args[1]);
            else
                throw cRuntimeError("Wrong number of args in '%s', scale(s) or scale(sx,sy) expected", step);
        }
        else if (operation == "skewx") {
            if (args.size() == 1)
                transform.skewx(deg2rad(args[0]));
            else if (args.size() == 2)
                transform.skewx(deg2rad(args[0]), args[1]);
            else
                throw cRuntimeError("Wrong number of args in '%s', skewx(deg) or skewx(deg,cy) expected", step);
        }
        else if (operation == "skewy") {
            if (args.size() == 1)
                transform.skewy(deg2rad(args[0]));
            else if (args.size() == 2)
                transform.skewy(deg2rad(args[0]), args[1]);
            else
                throw cRuntimeError("Wrong number of args in '%s', skewy(deg) or skewy(deg,cx) expected", step);
        }
        else {
            throw cRuntimeError("Invalid operation %s, translate, rotate, scale, skewx or skewy expected: '%s'", operation.c_str(), step);
        }
    }
    return transform;
}

cFigure::Font cFigure::parseFont(cProperty *property, const char *key)
{
    const char *typeface = property->getValue(key, 0);
    int size = opp_atol(opp_nulltoempty(property->getValue(key, 1)));
    if (size <= 0)
        size = 0;  // default size
    int flags = 0;
    cStringTokenizer tokenizer(property->getValue(key, 2));
    while (tokenizer.hasMoreTokens()) {
        const char *token = tokenizer.nextToken();
        if (!strcmp(token, "normal"))  /*no-op*/;
        else if (!strcmp(token, "bold"))
            flags |= FONT_BOLD;
        else if (!strcmp(token, "italic"))
            flags |= FONT_ITALIC;
        else if (!strcmp(token, "underline"))
            flags |= FONT_UNDERLINE;
        else
            throw cRuntimeError("unknown font style '%s'", token);
    }
    return Font(opp_nulltoempty(typeface), size, flags);
}

bool cFigure::parseBool(const char *s)
{
    if (!strcmp(s, "true"))
        return true;
    if (!strcmp(s, "false"))
        return false;
    throw cRuntimeError("invalid boolean value '%s'", s);
}

cFigure::Color cFigure::parseColor(const char *s)
{
	Color color;
	common::parseColor(s, color.red, color.green, color.blue);
	return color;
}

cFigure::LineStyle cFigure::parseLineStyle(const char *s)
{
    if (!strcmp(s,"solid")) return LINE_SOLID;
    if (!strcmp(s,"dotted")) return LINE_DOTTED;
    if (!strcmp(s,"dashed")) return LINE_DASHED;
    throw cRuntimeError("invalid line style '%s'", s);
}

cFigure::CapStyle cFigure::parseCapStyle(const char *s)
{
    if (!strcmp(s,"butt")) return CAP_BUTT;
    if (!strcmp(s,"square")) return CAP_SQUARE;
    if (!strcmp(s,"round")) return CAP_ROUND;
    throw cRuntimeError("invalid cap style '%s'", s);
}

cFigure::JoinStyle cFigure::parseJoinStyle(const char *s)
{
    if (!strcmp(s,"bevel")) return JOIN_BEVEL;
    if (!strcmp(s,"miter")) return JOIN_MITER;
    if (!strcmp(s,"round")) return JOIN_ROUND;
    throw cRuntimeError("invalid join style '%s'", s);
}

cFigure::FillRule cFigure::parseFillRule(const char *s)
{
    if (!strcmp(s,"evenodd")) return FILL_EVENODD;
    if (!strcmp(s,"nonzero")) return FILL_NONZERO;
    throw cRuntimeError("invalid fill rule '%s'", s);
}

cFigure::ArrowHead cFigure::parseArrowHead(const char *s)
{
    if (!strcmp(s,"none")) return ARROW_NONE;
    if (!strcmp(s,"simple")) return ARROW_SIMPLE;
    if (!strcmp(s,"triangle")) return ARROW_TRIANGLE;
    if (!strcmp(s,"barbed")) return ARROW_BARBED;
    throw cRuntimeError("invalid arrowhead style '%s'", s);
}

cFigure::Interpolation cFigure::parseInterpolation(const char *s)
{
    if (!strcmp(s,"none")) return INTERPOLATION_NONE;
    if (!strcmp(s,"fast")) return INTERPOLATION_FAST;
    if (!strcmp(s,"best")) return INTERPOLATION_BEST;
    throw cRuntimeError("invalid interpolation mode '%s'", s);
}


cFigure::Anchor cFigure::parseAnchor(const char *s)
{
    if (!strcmp(s,"c")) return ANCHOR_CENTER;
    if (!strcmp(s,"center")) return ANCHOR_CENTER;
    if (!strcmp(s,"n")) return ANCHOR_N;
    if (!strcmp(s,"e")) return ANCHOR_E;
    if (!strcmp(s,"s")) return ANCHOR_S;
    if (!strcmp(s,"w")) return ANCHOR_W;
    if (!strcmp(s,"nw")) return ANCHOR_NW;
    if (!strcmp(s,"ne")) return ANCHOR_NE;
    if (!strcmp(s,"se")) return ANCHOR_SE;
    if (!strcmp(s,"sw")) return ANCHOR_SW;
    if (!strcmp(s,"start")) return ANCHOR_BASELINE_START;
    if (!strcmp(s,"middle")) return ANCHOR_BASELINE_MIDDLE;
    if (!strcmp(s,"end")) return ANCHOR_BASELINE_END;
    throw cRuntimeError("invalid anchor '%s'", s);
}

cFigure::~cFigure()
{
    for (int i = 0; i < (int)children.size(); i++)
        dropAndDelete(children[i]);
    stringPool.release(tags);
}

void cFigure::parse(cProperty *property)
{
    validatePropertyKeys(property);

    const char *s;
    if ((s = property->getValue(PKEY_VISIBLE)) != nullptr)
        setVisible(parseBool(s));
    int numTags = property->getNumValues(PKEY_TAGS);
    if (numTags > 0) {
        std::string tags;
        for (int i = 0; i < numTags; i++)
            tags += std::string(" ") + property->getValue(PKEY_TAGS, i);
        setTags(tags.c_str());
    }
    if (property->getNumValues(PKEY_TRANSFORM) != 0)
        setTransform(parseTransform(property, PKEY_TRANSFORM));
}

void cFigure::validatePropertyKeys(cProperty *property) const
{
    const std::vector<const char *>& keys = property->getKeys();
    for (int i = 0; i < (int)keys.size(); i++) {
        const char *key = keys[i];
        if (!isAllowedPropertyKey(key)) {
            std::string allowedList = opp_join(getAllowedPropertyKeys(), ", ");
            throw cRuntimeError(this, "Unknown property key '%s'; supported keys are: %s, and any string starting with 'x-'",
                    key, allowedList.c_str());
        }
    }
}

bool cFigure::isAllowedPropertyKey(const char *key) const
{
    const char **allowedKeys = getAllowedPropertyKeys();
    for (const char **allowedKeyPtr = allowedKeys; *allowedKeyPtr; allowedKeyPtr++)
        if (strcmp(key, *allowedKeyPtr) == 0)
            return true;
    if (key[0] == 'x' && key[1] == '-')  // "x-*" keys can be added by the user
        return true;
    return false;
}

const char **cFigure::getAllowedPropertyKeys() const
{
    static const char *keys[] = { PKEY_TYPE, PKEY_VISIBLE, PKEY_TAGS, PKEY_CHILDZ, PKEY_TRANSFORM, nullptr};
    return keys;
}

void cFigure::concatArrays(const char **dest, const char **first, const char **second)
{
    while (*first)
        *dest++ = *first++;
    while (*second)
        *dest++ = *second++;
    *dest = nullptr;
}

cFigure *cFigure::dupTree() const
{
    cFigure *result = dup();
    for (int i = 0; i < (int)children.size(); i++)
        result->addFigure(children[i]->dupTree());
    return result;
}

void cFigure::copy(const cFigure& other)
{
    setVisible(other.isVisible());
    setTags(other.getTags());
    setTransform(other.getTransform());
}

cFigure& cFigure::operator=(const cFigure& other)
{
    if (this == &other)
        return *this;
    cOwnedObject::operator=(other);
    copy(other);
    return *this;
}

void cFigure::forEachChild(cVisitor *v)
{
    for (int i = 0; i < (int)children.size(); i++)
        v->visit(children[i]);
}

std::string cFigure::info() const
{
    return "";
}

void cFigure::setTags(const char *tags)
{
    if (omnetpp::opp_strcmp(this->tags, tags) == 0)
        return;
    const char *oldTags = this->tags;
    this->tags = stringPool.get(tags);
    stringPool.release(oldTags);
    refreshTagBits();
    fire(CHANGE_TAGS);
}

void cFigure::addFigure(cFigure *figure)
{
    if (!figure)
        throw cRuntimeError(this, "addFigure(): cannot insert nullptr");
    take(figure);
    children.push_back(figure);
    refreshTagBits();
    figure->clearChangeFlags();
    fireStructuralChange();
}

void cFigure::addFigure(cFigure *figure, int pos)
{
    if (!figure)
        throw cRuntimeError(this, "addFigure(): cannot insert nullptr");
    if (pos < 0 || pos > (int)children.size())
        throw cRuntimeError(this, "addFigure(): insert position %d out of bounds", pos);
    take(figure);
    children.insert(children.begin() + pos, figure);
    refreshTagBits();
    figure->clearChangeFlags();
    fireStructuralChange();
}

void cFigure::addFigureAbove(cFigure *figure, cFigure *referenceFigure)
{
    int refPos = findFigure(referenceFigure);
    if (refPos == -1)
        throw cRuntimeError(this, "addFigureAbove(): reference figure is not a child");
    addFigure(figure, refPos + 1);
}

void cFigure::addFigureBelow(cFigure *figure, cFigure *referenceFigure)
{
    int refPos = findFigure(referenceFigure);
    if (refPos == -1)
        throw cRuntimeError(this, "addFigureBelow(): reference figure is not a child");
    addFigure(figure, refPos);
}

inline double getZ(cFigure *figure, const std::map<cFigure *, double>& orderMap)
{
    const double defaultZ = 0.0;
    std::map<cFigure *, double>::const_iterator it = orderMap.find(figure);
    return (it == orderMap.end()) ? defaultZ : it->second;
}

struct LessZ
{
    std::map<cFigure *, double>& orderMap;
    LessZ(std::map<cFigure *, double>& orderMap) : orderMap(orderMap) {}
    bool operator()(cFigure *figure1, cFigure *figure2) { return getZ(figure1, orderMap) < getZ(figure2, orderMap); }
};

void cFigure::insertChild(cFigure *figure, std::map<cFigure *, double>& orderMap)
{
    // Assuming that existing children are z-ordered, insert a new child at the appropriate place.
    // Z-order comes from the orderMap; if a figure is not in the map, its Z is assumed to be zero.
    take(figure);
    std::vector<cFigure *>::iterator it = std::upper_bound(children.begin(), children.end(), figure, LessZ(orderMap));
    children.insert(it, figure);
    refreshTagBits();
    fireStructuralChange();
}

cFigure *cFigure::removeFromParent()
{
    cFigure *parent = getParentFigure();
    return !parent ? this : parent->removeFigure(this);
}

cFigure *cFigure::removeFigure(int pos)
{
    if (pos < 0 || pos >= (int)children.size())
        throw cRuntimeError(this, "removeFigure(): index %d out of bounds", pos);
    cFigure *figure = children[pos];
    children.erase(children.begin() + pos);
    drop(figure);
    fireStructuralChange();
    return figure;
}

cFigure *cFigure::removeFigure(cFigure *figure)
{
    int pos = findFigure(figure);
    if (pos == -1)
        throw cRuntimeError(this, "removeFigure(): figure is not a child");
    return removeFigure(pos);
}

int cFigure::findFigure(const char *name) const
{
    for (int i = 0; i < (int)children.size(); i++)
        if (children[i]->isName(name))
            return i;
    return -1;
}

int cFigure::findFigure(cFigure *figure) const
{
    for (int i = 0; i < (int)children.size(); i++)
        if (children[i] == figure)
            return i;
    return -1;
}

cFigure *cFigure::getFigure(int pos) const
{
    if (pos < 0 || pos >= (int)children.size())
        throw cRuntimeError(this, "getFigure(): index %d out of bounds", pos);
    return children[pos];
}

cFigure *cFigure::getFigure(const char *name) const
{
    for (int i = 0; i < (int)children.size(); i++) {
        cFigure *figure = children[i];
        if (figure->isName(name))
            return figure;
    }
    return nullptr;
}

cFigure *cFigure::findFigureRecursively(const char *name) const
{
    if (!strcmp(name, getFullName()))
        return const_cast<cFigure *>(this);
    for (int i = 0; i < (int)children.size(); i++) {
        cFigure *figure = children[i]->findFigureRecursively(name);
        if (figure)
            return figure;
    }
    return nullptr;
}

cCanvas *cFigure::getCanvas() const
{
    return dynamic_cast<cCanvas *>(getRootFigure()->getOwner());
}

static char *nextToken(char *& rest)
{
    if (!rest)
        return nullptr;
    char *token = rest;
    rest = strchr(rest, '.');
    if (rest)
        *rest++ = '\0';
    return token;
}

cFigure *cFigure::getRootFigure() const
{
    cFigure *figure = const_cast<cFigure *>(this);
    cFigure *parent;
    while ((parent = figure->getParentFigure()) != nullptr)
        figure = parent;
    return figure;
}

cFigure *cFigure::getFigureByPath(const char *path) const
{
    if (!path || !path[0])
        return nullptr;

    // determine starting point
    bool isRelative = (path[0] == '.' || path[0] == '^');
    cFigure *rootFigure = isRelative ? nullptr : getRootFigure();  // only needed when processing absolute paths
    cFigure *figure = isRelative ? const_cast<cFigure *>(this) : rootFigure;
    if (path[0] == '.')
        path++;  // skip initial dot

    // match components of the path
    opp_string pathbuf(path);
    char *rest = pathbuf.buffer();
    char *token = nextToken(rest);
    while (token && figure) {
        if (!token[0])
            ;  /*skip empty path component*/
        else if (token[0] == '^' && token[1] == '\0')
            figure = figure->getParentFigure();
        else
            figure = figure->getFigure(token);
        token = nextToken(rest);
    }
    return figure;  // nullptr if not found
}

void cFigure::refreshTagBits()
{
    cCanvas *canvas = getCanvas();
    if (canvas) {
        tagBits = canvas->parseTags(getTags());
        for (int i = 0; i < (int)children.size(); i++)
            children[i]->refreshTagBits();
    }
}

void cFigure::fire(uint8_t flags)
{
    if ((localChanges & flags) == 0) {  // not yet set
        localChanges |= flags;
        for (cFigure *figure = getParentFigure(); figure != nullptr; figure = figure->getParentFigure())
            figure->subtreeChanges |= flags;
    }
}

void cFigure::clearChangeFlags()
{
    if (subtreeChanges)
        for (int i = 0; i < (int)children.size(); i++)
            children[i]->clearChangeFlags();
    localChanges = subtreeChanges = 0;
}

void cFigure::raiseAbove(cFigure *figure)
{
    cFigure *parent = getParentFigure();
    if (!parent)
        throw cRuntimeError(this, "raiseAbove(): figure has no parent figure");
    int myPos = parent->findFigure(this);
    int refPos = parent->findFigure(figure);
    if (refPos == -1)
        throw cRuntimeError(this, "raiseAbove(): reference figure must have the same parent");
    if (myPos < refPos) {
        parent->children.erase(parent->children.begin() + myPos);  // note: reference figure will be shifted down
        parent->children.insert(parent->children.begin() + refPos, this);
        fireStructuralChange();
    }
}

void cFigure::lowerBelow(cFigure *figure)
{
    cFigure *parent = getParentFigure();
    if (!parent)
        throw cRuntimeError(this, "lowerBelow(): figure has no parent figure");
    int myPos = parent->findFigure(this);
    int refPos = parent->findFigure(figure);
    if (refPos == -1)
        throw cRuntimeError(this, "lowerBelow(): reference figure must have the same parent");
    if (myPos > refPos) {
        parent->children.erase(parent->children.begin() + myPos);
        parent->children.insert(parent->children.begin() + refPos, this);
        fireStructuralChange();
    }
}

void cFigure::raiseToTop()
{
    cFigure *parent = getParentFigure();
    if (!parent)
        throw cRuntimeError(this, "raiseToTop(): figure has no parent figure");
    int myPos = parent->findFigure(this);
    if (myPos != (int)parent->children.size() - 1) {
        parent->children.erase(parent->children.begin() + myPos);
        parent->children.push_back(this);
        fireStructuralChange();
    }
}

void cFigure::lowerToBottom()
{
    cFigure *parent = getParentFigure();
    if (!parent)
        throw cRuntimeError(this, "lowerToBottom(): figure has no parent figure");
    int myPos = parent->findFigure(this);
    if (myPos != 0) {
        parent->children.erase(parent->children.begin() + myPos);
        parent->children.insert(parent->children.begin(), this);
        fireStructuralChange();
    }
}

//----

cGroupFigure& cGroupFigure::operator=(const cGroupFigure& other)
{
    if (this == &other)
        return *this;
    cFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cGroupFigure::info() const
{
    return "";
}

//----

#if 0
void cPanelFigure::copy(const cPanelFigure& other)
{
    setPosition(other.getPosition());
}

cPanelFigure& cPanelFigure::operator=(const cPanelFigure& other)
{
    if (this == &other)
        return *this;
    cFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cPanelFigure::info() const
{
    std::stringstream os;
    os << "at " << getPosition();
    return os.str();
}

void cPanelFigure::parse(cProperty *property)
{
    cFigure::parse(property);
    setPosition(parsePoint(property, PKEY_POS, 0));
}

const char **cPanelFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POS, nullptr};
        concatArrays(keys, cFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cPanelFigure::updateParentTransform(Transform& transform)
{
    // replace current transform with an axis-aligned, unscaled (thus also unzoomable)
    // coordinate system, with the origin at getPosition()
    Point origin = transform.applyTo(getPosition());
    transform = Transform().translate(origin.x, origin.y);

    // then apply our own transform in the normal way (like all other figures do)
    transform.leftMultiply(getTransform());
}
#endif

//----

void cAbstractLineFigure::copy(const cAbstractLineFigure& other)
{
    setLineColor(other.getLineColor());
    setLineStyle(other.getLineStyle());
    setLineWidth(other.getLineWidth());
    setLineOpacity(other.getLineOpacity());
    setCapStyle(other.getCapStyle());
    setStartArrowHead(other.getStartArrowHead());
    setEndArrowHead(other.getEndArrowHead());
}

cAbstractLineFigure& cAbstractLineFigure::operator=(const cAbstractLineFigure& other)
{
    if (this == &other)
        return *this;
    cFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cAbstractLineFigure::info() const
{
    return "";
}

void cAbstractLineFigure::parse(cProperty *property)
{
    cFigure::parse(property);
    const char *s;
    if ((s = property->getValue(PKEY_LINECOLOR)) != nullptr)
        setLineColor(parseColor(s));
    if ((s = property->getValue(PKEY_LINESTYLE)) != nullptr)
        setLineStyle(parseLineStyle(s));
    if ((s = property->getValue(PKEY_LINEWIDTH)) != nullptr)
        setLineWidth(opp_atof(s));
    if ((s = property->getValue(PKEY_LINEOPACITY)) != nullptr)
        setLineOpacity(opp_atof(s));
    if ((s = property->getValue(PKEY_CAPSTYLE, 0)) != nullptr)
        setCapStyle(parseCapStyle(s));
    if ((s = property->getValue(PKEY_STARTARROWHEAD)) != nullptr)
        setStartArrowHead(parseArrowHead(s));
    if ((s = property->getValue(PKEY_ENDARROWHEAD)) != nullptr)
        setEndArrowHead(parseArrowHead(s));
    if ((s = property->getValue(PKEY_ZOOMLINEWIDTH)) != nullptr)
        setZoomLineWidth(parseBool(s));
}

const char **cAbstractLineFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_LINECOLOR, PKEY_LINESTYLE, PKEY_LINEWIDTH, PKEY_LINEOPACITY, PKEY_CAPSTYLE, PKEY_STARTARROWHEAD, PKEY_ENDARROWHEAD, PKEY_ZOOMLINEWIDTH, nullptr};
        concatArrays(keys, cFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cAbstractLineFigure::setLineColor(const Color& lineColor)
{
    if (lineColor == this->lineColor)
        return;
    this->lineColor = lineColor;
    fireVisualChange();
}

void cAbstractLineFigure::setLineWidth(double lineWidth)
{
    if (lineWidth == this->lineWidth)
        return;
    ENSURE_POSITIVE(lineWidth);
    this->lineWidth = lineWidth;
    fireVisualChange();
}

void cAbstractLineFigure::setLineOpacity(double lineOpacity)
{
    if (lineOpacity == this->lineOpacity)
        return;
    ENSURE_RANGE01(lineOpacity);
    this->lineOpacity = lineOpacity;
    fireVisualChange();
}

void cAbstractLineFigure::setLineStyle(LineStyle lineStyle)
{
    if (lineStyle == this->lineStyle)
        return;
    this->lineStyle = lineStyle;
    fireVisualChange();
}

void cAbstractLineFigure::setCapStyle(CapStyle capStyle)
{
    if (capStyle == this->capStyle)
        return;
    this->capStyle = capStyle;
    fireVisualChange();
}

void cAbstractLineFigure::setStartArrowHead(ArrowHead startArrowHead)
{
    if (startArrowHead == this->startArrowHead)
        return;
    this->startArrowHead = startArrowHead;
    fireVisualChange();
}

void cAbstractLineFigure::setEndArrowHead(ArrowHead endArrowHead)
{
    if (endArrowHead == this->endArrowHead)
        return;
    this->endArrowHead = endArrowHead;
    fireVisualChange();
}

void cAbstractLineFigure::setZoomLineWidth(bool zoomLineWidth)
{
    if (zoomLineWidth == this->zoomLineWidth)
        return;
    this->zoomLineWidth = zoomLineWidth;
    fireVisualChange();
}

//----

void cLineFigure::copy(const cLineFigure& other)
{
    setStart(other.getStart());
    setEnd(other.getEnd());
}

cLineFigure& cLineFigure::operator=(const cLineFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractLineFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cLineFigure::info() const
{
    std::stringstream os;
    os << getStart() << " to " << getEnd();
    return os.str();
}

void cLineFigure::parse(cProperty *property)
{
    cAbstractLineFigure::parse(property);

    setStart(parsePoint(property, PKEY_POINTS, 0));
    setEnd(parsePoint(property, PKEY_POINTS, 2));
}

const char **cLineFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POINTS, nullptr};
        concatArrays(keys, cAbstractLineFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cLineFigure::move(double x, double y)
{
    start.x += x;
    start.y += y;
    end.x += x;
    end.y += y;
    fireGeometryChange();
}

void cLineFigure::setStart(const Point& start)
{
    if (start == this->start)
        return;
    this->start = start;
    fireGeometryChange();
}

void cLineFigure::setEnd(const Point& end)
{
    if (end == this->end)
        return;
    this->end = end;
    fireGeometryChange();
}

//----

void cArcFigure::copy(const cArcFigure& other)
{
    setBounds(other.getBounds());
    setStartAngle(other.getStartAngle());
    setEndAngle(other.getEndAngle());
}

cArcFigure& cArcFigure::operator=(const cArcFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractLineFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cArcFigure::info() const
{
    std::stringstream os;
    os << getBounds() << ", " << floor(rad2deg(getStartAngle())) << " to " << floor(rad2deg(getEndAngle())) << " degrees";
    return os.str();
}

void cArcFigure::parse(cProperty *property)
{
    cAbstractLineFigure::parse(property);

    setBounds(parseBounds(property));

    const char *s;
    if ((s = property->getValue(PKEY_STARTANGLE)) != nullptr)
        setStartAngle(deg2rad(opp_atof(s)));
    if ((s = property->getValue(PKEY_ENDANGLE)) != nullptr)
        setEndAngle(deg2rad(opp_atof(s)));
}

const char **cArcFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_STARTANGLE, PKEY_ENDANGLE, nullptr};
        concatArrays(keys, cAbstractLineFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cArcFigure::move(double x, double y)
{
    bounds.x += x;
    bounds.y += y;
    fireGeometryChange();
}

void cArcFigure::setBounds(const Rectangle& bounds)
{
    if (bounds == this->bounds)
        return;
    ENSURE_NONNEGATIVE(bounds.width);
    ENSURE_NONNEGATIVE(bounds.height);
    this->bounds = bounds;
    fireGeometryChange();
}

void cArcFigure::setStartAngle(double startAngle)
{
    if (startAngle == this->startAngle)
        return;
    this->startAngle = startAngle;
    fireGeometryChange();
}

void cArcFigure::setEndAngle(double endAngle)
{
    if (endAngle == this->endAngle)
        return;
    this->endAngle = endAngle;
    fireGeometryChange();
}

//----

void cPolylineFigure::copy(const cPolylineFigure& other)
{
    setPoints(other.getPoints());
    setSmooth(other.getSmooth());
    setJoinStyle(other.getJoinStyle());
}

void cPolylineFigure::checkIndex(int i) const
{
    if (i < 0 || i >= (int)points.size())
        throw cRuntimeError(this, "Index %d is out of range", i);
}

void cPolylineFigure::checkInsIndex(int i) const
{
    if (i < 0 || i > (int)points.size())
        throw cRuntimeError(this, "Insertion index %d is out of range", i);
}

cPolylineFigure& cPolylineFigure::operator=(const cPolylineFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractLineFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cPolylineFigure::info() const
{
    std::stringstream os;
    for (int i = 0; i < (int)points.size(); i++)
        os << points[i] << " ";
    return os.str();
}

void cPolylineFigure::parse(cProperty *property)
{
    cAbstractLineFigure::parse(property);

    const char *s;
    setPoints(parsePoints(property, PKEY_POINTS));
    if ((s = property->getValue(PKEY_SMOOTH, 0)) != nullptr)
        setSmooth(parseBool(s));
    if ((s = property->getValue(PKEY_JOINSTYLE, 0)) != nullptr)
        setJoinStyle(parseJoinStyle(s));
}

const char **cPolylineFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POINTS, PKEY_SMOOTH, PKEY_JOINSTYLE, nullptr};
        concatArrays(keys, cAbstractLineFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cPolylineFigure::move(double x, double y)
{
    for (int i = 0; i < (int)points.size(); i++) {
        points[i].x += x;
        points[i].y += y;
    }
    fireGeometryChange();
}

void cPolylineFigure::setPoints(const std::vector<Point>& points)
{
    if (points == this->points)
        return;
    this->points = points;
    fireGeometryChange();
}

void cPolylineFigure::setPoint(int i, const Point& point)
{
    checkIndex(i);
    if (point == this->points[i])
        return;
    this->points[i] = point;
    fireGeometryChange();
}

void cPolylineFigure::addPoint(const Point& point)
{
    this->points.push_back(point);
    fireGeometryChange();
}

void cPolylineFigure::removePoint(int i)
{
    checkIndex(i);
    this->points.erase(this->points.begin() + i);
    fireGeometryChange();
}

void cPolylineFigure::insertPoint(int i, const Point& point)
{
    checkInsIndex(i);
    this->points.insert(this->points.begin() + i, point);
    fireGeometryChange();
}

void cPolylineFigure::setSmooth(bool smooth)
{
    if (smooth == this->smooth)
        return;
    this->smooth = smooth;
    fireVisualChange();
}

void cPolylineFigure::setJoinStyle(JoinStyle joinStyle)
{
    if (joinStyle == this->joinStyle)
        return;
    this->joinStyle = joinStyle;
    fireVisualChange();
}

//----

void cAbstractShapeFigure::copy(const cAbstractShapeFigure& other)
{
    setOutlined(other.isOutlined());
    setFilled(other.isFilled());
    setLineColor(other.getLineColor());
    setFillColor(other.getFillColor());
    setLineStyle(other.getLineStyle());
    setLineWidth(other.getLineWidth());
    setLineOpacity(other.getLineOpacity());
    setFillOpacity(other.getFillOpacity());
}

cAbstractShapeFigure& cAbstractShapeFigure::operator=(const cAbstractShapeFigure& other)
{
    if (this == &other)
        return *this;
    cFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cAbstractShapeFigure::info() const
{
    return "";
}

void cAbstractShapeFigure::parse(cProperty *property)
{
    cFigure::parse(property);

    const char *s;
    if ((s = property->getValue(PKEY_LINECOLOR)) != nullptr) {
        if (opp_isblank(s))
            setOutlined(false);
        else
            setLineColor(parseColor(s));
    }
    if ((s = property->getValue(PKEY_FILLCOLOR)) != nullptr) {
        setFillColor(parseColor(s));
        setFilled(true);
    }
    if ((s = property->getValue(PKEY_LINESTYLE)) != nullptr)
        setLineStyle(parseLineStyle(s));
    if ((s = property->getValue(PKEY_LINEWIDTH)) != nullptr)
        setLineWidth(opp_atof(s));
    if ((s = property->getValue(PKEY_LINEOPACITY)) != nullptr)
        setLineOpacity(opp_atof(s));
    if ((s = property->getValue(PKEY_FILLOPACITY)) != nullptr)
        setFillOpacity(opp_atof(s));
    if ((s = property->getValue(PKEY_ZOOMLINEWIDTH)) != nullptr)
        setZoomLineWidth(parseBool(s));
}

const char **cAbstractShapeFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_LINECOLOR, PKEY_FILLCOLOR, PKEY_LINESTYLE, PKEY_LINEWIDTH, PKEY_LINEOPACITY, PKEY_FILLOPACITY, PKEY_ZOOMLINEWIDTH, nullptr};
        concatArrays(keys, cFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cAbstractShapeFigure::setFilled(bool filled)
{
    if (filled == this->filled)
        return;
    this->filled = filled;
    fireVisualChange();
}

void cAbstractShapeFigure::setOutlined(bool outlined)
{
    if (outlined == this->outlined)
        return;
    this->outlined = outlined;
    fireVisualChange();
}

void cAbstractShapeFigure::setLineColor(const Color& lineColor)
{
    if (lineColor == this->lineColor)
        return;
    this->lineColor = lineColor;
    fireVisualChange();
}

void cAbstractShapeFigure::setFillColor(const Color& fillColor)
{
    if (fillColor == this->fillColor)
        return;
    this->fillColor = fillColor;
    fireVisualChange();
}

void cAbstractShapeFigure::setLineStyle(LineStyle lineStyle)
{
    if (lineStyle == this->lineStyle)
        return;
    this->lineStyle = lineStyle;
    fireVisualChange();
}

void cAbstractShapeFigure::setLineWidth(double lineWidth)
{
    if (lineWidth == this->lineWidth)
        return;
    ENSURE_POSITIVE(lineWidth);
    this->lineWidth = lineWidth;
    fireVisualChange();
}

void cAbstractShapeFigure::setLineOpacity(double lineOpacity)
{
    if (lineOpacity == this->lineOpacity)
        return;
    ENSURE_RANGE01(lineOpacity);
    this->lineOpacity = lineOpacity;
    fireVisualChange();
}

void cAbstractShapeFigure::setFillOpacity(double fillOpacity)
{
    if (fillOpacity == this->fillOpacity)
        return;
    ENSURE_RANGE01(fillOpacity);
    this->fillOpacity = fillOpacity;
    fireVisualChange();
}

void cAbstractShapeFigure::setZoomLineWidth(bool zoomLineWidth)
{
    if (zoomLineWidth == this->zoomLineWidth)
        return;
    this->zoomLineWidth = zoomLineWidth;
    fireVisualChange();
}

//----

void cRectangleFigure::copy(const cRectangleFigure& other)
{
    setBounds(other.getBounds());
    setCornerRx(other.getCornerRx());
    setCornerRy(other.getCornerRy());
}

cRectangleFigure& cRectangleFigure::operator=(const cRectangleFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractShapeFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cRectangleFigure::info() const
{
    std::stringstream os;
    os << getBounds();
    return os.str();
}

void cRectangleFigure::parse(cProperty *property)
{
    cAbstractShapeFigure::parse(property);

    setBounds(parseBounds(property));
    const char *s;
    if ((s = property->getValue(PKEY_CORNERRADIUS, 0)) != nullptr) {
        setCornerRx(opp_atof(s));
        setCornerRy(opp_atof(s));  // as default
    }
    if ((s = property->getValue(PKEY_CORNERRADIUS, 1)) != nullptr)
        setCornerRy(opp_atof(s));
}

const char **cRectangleFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_CORNERRADIUS, nullptr};
        concatArrays(keys, cAbstractShapeFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cRectangleFigure::move(double x, double y)
{
    bounds.x += x;
    bounds.y += y;
    fireGeometryChange();
}

void cRectangleFigure::setBounds(const Rectangle& bounds)
{
    if (bounds == this->bounds)
        return;
    ENSURE_NONNEGATIVE(bounds.width);
    ENSURE_NONNEGATIVE(bounds.height);
    this->bounds = bounds;
    fireGeometryChange();
}

void cRectangleFigure::setCornerRx(double cornerRx)
{
    if (cornerRx == this->cornerRx)
        return;
    ENSURE_NONNEGATIVE(cornerRx);
    this->cornerRx = cornerRx;
    fireGeometryChange();
}

void cRectangleFigure::setCornerRy(double cornerRy)
{
    if (cornerRy == this->cornerRy)
        return;
    ENSURE_NONNEGATIVE(cornerRy);
    this->cornerRy = cornerRy;
    fireGeometryChange();
}

//----

void cOvalFigure::copy(const cOvalFigure& other)
{
    setBounds(other.getBounds());
}

cOvalFigure& cOvalFigure::operator=(const cOvalFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractShapeFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cOvalFigure::info() const
{
    std::stringstream os;
    os << getBounds();
    return os.str();
}

void cOvalFigure::parse(cProperty *property)
{
    cAbstractShapeFigure::parse(property);

    setBounds(parseBounds(property));
}

const char **cOvalFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, nullptr};
        concatArrays(keys, cAbstractShapeFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cOvalFigure::move(double x, double y)
{
    bounds.x += x;
    bounds.y += y;
    fireGeometryChange();
}

void cOvalFigure::setBounds(const Rectangle& bounds)
{
    if (bounds == this->bounds)
        return;
    ENSURE_NONNEGATIVE(bounds.width);
    ENSURE_NONNEGATIVE(bounds.height);
    this->bounds = bounds;
    fireGeometryChange();
}

//----

void cRingFigure::copy(const cRingFigure& other)
{
    setBounds(other.getBounds());
    setInnerRx(other.getInnerRx());
    setInnerRy(other.getInnerRy());
}

cRingFigure& cRingFigure::operator=(const cRingFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractShapeFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cRingFigure::info() const
{
    std::stringstream os;
    os << getBounds();
    return os.str();
}

void cRingFigure::parse(cProperty *property)
{
    cAbstractShapeFigure::parse(property);

    setBounds(parseBounds(property));
    const char *s;
    if ((s = property->getValue(PKEY_INNERSIZE, 0)) != nullptr) {
        setInnerRx(opp_atof(s) / 2);
        setInnerRy(opp_atof(s) / 2);  // as default
    }
    if ((s = property->getValue(PKEY_INNERSIZE, 1)) != nullptr)
        setInnerRy(opp_atof(s) / 2);
}

const char **cRingFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_INNERSIZE, nullptr};
        concatArrays(keys, cAbstractShapeFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cRingFigure::move(double x, double y)
{
    bounds.x += x;
    bounds.y += y;
    fireGeometryChange();
}

void cRingFigure::setBounds(const Rectangle& bounds)
{
    if (bounds == this->bounds)
        return;
    ENSURE_NONNEGATIVE(bounds.width);
    ENSURE_NONNEGATIVE(bounds.height);
    this->bounds = bounds;
    fireGeometryChange();
}

void cRingFigure::setInnerRx(double innerRx)
{
    if (innerRx == this->innerRx)
        return;
    ENSURE_NONNEGATIVE(innerRx);
    this->innerRx = innerRx;
    fireGeometryChange();
}

void cRingFigure::setInnerRy(double innerRy)
{
    if (innerRy == this->innerRy)
        return;
    ENSURE_NONNEGATIVE(innerRy);
    this->innerRy = innerRy;
    fireGeometryChange();
}

//----

void cPieSliceFigure::copy(const cPieSliceFigure& other)
{
    setBounds(other.getBounds());
    setStartAngle(other.getStartAngle());
    setEndAngle(other.getEndAngle());
}

cPieSliceFigure& cPieSliceFigure::operator=(const cPieSliceFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractShapeFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cPieSliceFigure::info() const
{
    std::stringstream os;
    os << getBounds();
    return os.str();
}

void cPieSliceFigure::parse(cProperty *property)
{
    cAbstractShapeFigure::parse(property);

    setBounds(parseBounds(property));

    const char *s;
    if ((s = property->getValue(PKEY_STARTANGLE)) != nullptr)
        setStartAngle(deg2rad(opp_atof(s)));
    if ((s = property->getValue(PKEY_ENDANGLE)) != nullptr)
        setEndAngle(deg2rad(opp_atof(s)));
}

const char **cPieSliceFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_STARTANGLE, PKEY_ENDANGLE, nullptr};
        concatArrays(keys, cAbstractShapeFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cPieSliceFigure::move(double x, double y)
{
    bounds.x += x;
    bounds.y += y;
    fireGeometryChange();
}

void cPieSliceFigure::setBounds(const Rectangle& bounds)
{
    if (bounds == this->bounds)
        return;
    ENSURE_NONNEGATIVE(bounds.width);
    ENSURE_NONNEGATIVE(bounds.height);
    this->bounds = bounds;
    fireGeometryChange();
}

void cPieSliceFigure::setStartAngle(double startAngle)
{
    if (startAngle == this->startAngle)
        return;
    this->startAngle = startAngle;
    fireGeometryChange();
}

void cPieSliceFigure::setEndAngle(double endAngle)
{
    if (endAngle == this->endAngle)
        return;
    this->endAngle = endAngle;
    fireGeometryChange();
}

//----

void cPolygonFigure::copy(const cPolygonFigure& other)
{
    setPoints(other.getPoints());
    setSmooth(other.getSmooth());
    setJoinStyle(other.getJoinStyle());
}

void cPolygonFigure::checkIndex(int i) const
{
    if (i < 0 || i >= (int)points.size())
        throw cRuntimeError(this, "Index %d is out of range", i);
}

void cPolygonFigure::checkInsIndex(int i) const
{
    if (i < 0 || i > (int)points.size())
        throw cRuntimeError(this, "Insertion index %d is out of range", i);
}

cPolygonFigure& cPolygonFigure::operator=(const cPolygonFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractShapeFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cPolygonFigure::info() const
{
    std::stringstream os;
    for (int i = 0; i < (int)points.size(); i++)
        os << points[i] << " ";
    return os.str();
}

void cPolygonFigure::parse(cProperty *property)
{
    cAbstractShapeFigure::parse(property);

    const char *s;
    setPoints(parsePoints(property, PKEY_POINTS));
    if ((s = property->getValue(PKEY_SMOOTH, 0)) != nullptr)
        setSmooth(parseBool(s));
    if ((s = property->getValue(PKEY_JOINSTYLE, 0)) != nullptr)
        setJoinStyle(parseJoinStyle(s));
    if ((s = property->getValue(PKEY_FILLRULE, 0)) != nullptr)
        setFillRule(parseFillRule(s));
}

const char **cPolygonFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POINTS, PKEY_SMOOTH, PKEY_JOINSTYLE, PKEY_FILLRULE, nullptr };
        concatArrays(keys, cAbstractShapeFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cPolygonFigure::move(double x, double y)
{
    for (int i = 0; i < (int)points.size(); i++) {
        points[i].x += x;
        points[i].y += y;
    }
    fireGeometryChange();
}

void cPolygonFigure::setPoints(const std::vector<Point>& points)
{
    if (points == this->points)
        return;
    this->points = points;
    fireGeometryChange();
}

void cPolygonFigure::setPoint(int i, const Point& point)
{
    checkIndex(i);
    if (point == this->points[i])
        return;
    this->points[i] = point;
    fireGeometryChange();
}

void cPolygonFigure::addPoint(const Point& point)
{
    this->points.push_back(point);
    fireGeometryChange();
}

void cPolygonFigure::removePoint(int i)
{
    checkIndex(i);
    this->points.erase(this->points.begin() + i);
    fireGeometryChange();
}

void cPolygonFigure::insertPoint(int i, const Point& point)
{
    checkInsIndex(i);
    this->points.insert(this->points.begin() + i, point);
    fireGeometryChange();
}

void cPolygonFigure::setSmooth(bool smooth)
{
    if (smooth == this->smooth)
        return;
    this->smooth = smooth;
    fireVisualChange();
}

void cPolygonFigure::setJoinStyle(JoinStyle joinStyle)
{
    if (joinStyle == this->joinStyle)
        return;
    this->joinStyle = joinStyle;
    fireVisualChange();
}

void cPolygonFigure::setFillRule(FillRule fillRule)
{
    if (fillRule == this->fillRule)
        return;
    this->fillRule = fillRule;
    fireVisualChange();
}

//----

void cPathFigure::copy(const cPathFigure& other)
{
    setPath(other.getPath()); //FIXME do deep copy of structs instead!!!
    setJoinStyle(other.getJoinStyle());
    setCapStyle(other.getCapStyle());
}

cPathFigure& cPathFigure::operator=(const cPathFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractShapeFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cPathFigure::info() const
{
    return getPath();
}

void cPathFigure::parse(cProperty *property)
{
    cAbstractShapeFigure::parse(property);

    const char *s;
    if ((s = property->getValue(PKEY_PATH, 0)) != nullptr)
        setPath(s);
    if ((s = property->getValue(PKEY_OFFSET, 0)) != nullptr)
        setOffset(parsePoint(property, s, 0));
    if ((s = property->getValue(PKEY_JOINSTYLE, 0)) != nullptr)
        setJoinStyle(parseJoinStyle(s));
    if ((s = property->getValue(PKEY_CAPSTYLE, 0)) != nullptr)
        setCapStyle(parseCapStyle(s));
    if ((s = property->getValue(PKEY_FILLRULE, 0)) != nullptr)
        setFillRule(parseFillRule(s));
}

const char **cPathFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_PATH, PKEY_OFFSET, PKEY_JOINSTYLE, PKEY_CAPSTYLE, PKEY_FILLRULE, nullptr};
        concatArrays(keys, cAbstractShapeFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

static double getNum(const char *&s)
{
    char *end;
    double d = opp_strtod(s, &end);
    if (end == s)
        throw cRuntimeError("number expected");
    s = end;
    return d;
}

static bool getBool(const char *&s)
{
    double d = getNum(s);
    if (d != 0 && d != 1)
        throw cRuntimeError("boolean (0 or 1) expected");
    return d != 0;
}

void cPathFigure::setPath(const char *pathString)
{
    clearPath();

    const char *s = pathString;
    while (opp_isspace(*s))
        s++;
    try {
        while (*s) {
            char code = *s++;
            switch (code) {
                case 'M': {
                    double x, y;
                    x = getNum(s); y = getNum(s);
                    addMoveTo(x, y);
                    break;
                }
                case 'm': {
                    double dx, dy;
                    dx = getNum(s); dy = getNum(s);
                    addMoveRel(dx, dy);
                    break;
                }
                case 'L': {
                    double x, y;
                    x = getNum(s); y = getNum(s);
                    addLineTo(x, y);
                    break;
                }
                case 'l': {
                    double dx, dy;
                    dx = getNum(s); dy = getNum(s);
                    addLineRel(dx, dy);
                    break;
                }
                case 'H': {
                    double x = getNum(s);
                    addHorizontalLineTo(x);
                    break;
                }
                case 'h': {
                    double dx = getNum(s);
                    addHorizontalLineRel(dx);
                    break;
                }
                case 'V': {
                    double y = getNum(s);
                    addVerticalLineTo(y);
                    break;
                }
                case 'v': {
                    double dy = getNum(s);
                    addVerticalLineRel(dy);
                    break;
                }
                case 'A': {
                    double rx, ry, phi, x, y;
                    bool sweep, largeArc;
                    rx = getNum(s); ry = getNum(s); phi = getNum(s); largeArc = getBool(s); sweep = getBool(s); x = getNum(s); y = getNum(s);
                    addArcTo(rx, ry, phi, largeArc, sweep, x, y);
                    break;
                }
                case 'a': {
                    double rx, ry, phi, dx, dy;
                    bool sweep, largeArc;
                    rx = getNum(s); ry = getNum(s); phi = getNum(s); largeArc = getBool(s); sweep = getBool(s); dx = getNum(s); dy = getNum(s);
                    addArcRel(rx, ry, phi, largeArc, sweep, dx, dy);
                    break;
                }
                case 'Q': {
                    double x1, y1, x, y;
                    x1 = getNum(s); y1 = getNum(s); x = getNum(s); y = getNum(s);
                    addCurveTo(x1, y1, x, y);
                    break;
                }
                case 'q': {
                    double dx1, dy1, dx, dy;
                    dx1 = getNum(s); dy1 = getNum(s); dx = getNum(s); dy = getNum(s);
                    addCurveRel(dx1, dy1, dx, dy);
                    break;
                }
                case 'T': {
                    double x, y;
                    x = getNum(s); y = getNum(s);
                    addSmoothCurveTo(x, y);
                    break;
                }
                case 't': {
                    double dx, dy;
                    dx = getNum(s); dy = getNum(s);
                    addSmoothCurveRel(dx, dy);
                    break;
                }
                case 'C': {
                    double x1, y1, x2, y2, x, y;
                    x1 = getNum(s); y1 = getNum(s); x2 = getNum(s); y2 = getNum(s); x = getNum(s); y = getNum(s);
                    addCubicBezierCurveTo(x1, y1, x2, y2, x, y);
                    break;
                }
                case 'c': {
                    double dx1, dy1, dx2, dy2, dx, dy;
                    dx1 = getNum(s); dy1 = getNum(s); dx2 = getNum(s); dy2 = getNum(s); dx = getNum(s); dy = getNum(s);
                    addCubicBezierCurveRel(dx1, dy1, dx2, dy2, dx, dy);
                    break;
                }
                case 'S': {
                    double x2, y2, x, y;
                    x2 = getNum(s); y2 = getNum(s); x = getNum(s); y = getNum(s);
                    addSmoothCubicBezierCurveTo(x2, y2, x, y);
                    break;
                }
                case 's': {
                    double dx2, dy2, dx, dy;
                    dx2 = getNum(s); dy2 = getNum(s); dx = getNum(s); dy = getNum(s);
                    addSmoothCubicBezierCurveRel(dx2, dy2, dx, dy);
                    break;
                }
                case 'z': case 'Z': {
                    addClosePath();
                    break;
                }
                default:
                    throw cRuntimeError("unknown drawing primitive '%c'", code);
            }
            while (opp_isspace(*s))
                s++;
        }
    }
    catch (std::exception& e) {
        std::string msg = opp_stringf("%s in path near column %d", e.what(), s - pathString);
        throw cRuntimeError(msg.c_str());
    }
}

const char *cPathFigure::getPath() const
{
    // return cached copy if exists
    if (!cachedPathString.empty() || path.empty())
        return cachedPathString.c_str();

    // else produce string and cache it
    std::stringstream os;
    for (int i = 0; i < (int)path.size(); i++) {
        PathItem *base = path[i];
        os << (char)base->code << " ";
        switch (base->code) {
            case 'M': {
                MoveTo *item = static_cast<MoveTo*>(base);
                os << item->x << " " << item->y;
                break;
            }
            case 'm': {
                MoveRel *item = static_cast<MoveRel*>(base);
                os << item->dx << " " << item->dy;
                break;
            }
            case 'L': {
                LineTo *item = static_cast<LineTo*>(base);
                os << item->x << " " << item->y;
                break;
            }
            case 'l': {
                LineRel *item = static_cast<LineRel*>(base);
                os << item->dx << " " << item->dy;
                break;
            }
            case 'H': {
                HorizLineTo *item = static_cast<HorizLineTo*>(base);
                os << item->x;
                break;
            }
            case 'h': {
                HorizLineRel *item = static_cast<HorizLineRel*>(base);
                os << item->dx;
                break;
            }
            case 'V': {
                VertLineTo *item = static_cast<VertLineTo*>(base);
                os << item->y;
                break;
            }
            case 'v': {
                VertLineRel *item = static_cast<VertLineRel*>(base);
                os << item->dy;
                break;
            }
            case 'A': {
                ArcTo *item = static_cast<ArcTo*>(base);
                os << item->rx << " " << item->ry << " " << item->phi << " " << item->largeArc << " " << item->sweep << " " << item->x << " " << item->y;
                break;
            }
            case 'a': {
                ArcRel *item = static_cast<ArcRel*>(base);
                os << item->rx << " " << item->ry << " " << item->phi << " " << item->largeArc << " " << item->sweep << " " << item->dx << " " << item->dy;
                break;
            }
            case 'Q': {
                CurveTo *item = static_cast<CurveTo*>(base);
                os << item->x1 << " " << item->y1 << " " << item->x << " " << item->y;
                break;
            }
            case 'q': {
                CurveRel *item = static_cast<CurveRel*>(base);
                os << item->dx1 << " " << item->dy1 << " " << item->dx << " " << item->dy;
                break;
            }
            case 'T': {
                SmoothCurveTo *item = static_cast<SmoothCurveTo*>(base);
                os << item->x << " " << item->y;
                break;
            }
            case 't': {
                SmoothCurveRel *item = static_cast<SmoothCurveRel*>(base);
                os << item->dx << " " << item->dy;
                break;
            }
            case 'C': {
                CubicBezierCurveTo *item = static_cast<CubicBezierCurveTo*>(base);
                os << item->x1 << " " << item->y1 << " " << item->x2 << " " << item->y2 << " " << item->x << " " << item->y;
                break;
            }
            case 'c': {
                CubicBezierCurveRel *item = static_cast<CubicBezierCurveRel*>(base);
                os << item->dx1 << " " << item->dy1 << " " << item->dx2 << " " << item->dy2 << " " << item->dx << " " << item->dy;
                break;
            }
            case 'S': {
                SmoothCubicBezierCurveTo *item = static_cast<SmoothCubicBezierCurveTo*>(base);
                os << item->x2 << " " << item->y2 << " " << item->x << " " << item->y;
                break;
            }
            case 's': {
                SmoothCubicBezierCurveRel *item = static_cast<SmoothCubicBezierCurveRel*>(base);
                os << item->dx2 << " " << item->dy2 << " " << item->dx << " " << item->dy;
                break;
            }
            case 'Z': {
                break;
            }
            default:
                throw cRuntimeError(this, "unknown path item '%c'", base->code);
        }
        os << " ";
    }
    cachedPathString = os.str();
    return cachedPathString.c_str();
}

void cPathFigure::addItem(PathItem *item)
{
    path.push_back(item);
    if (!cachedPathString.empty())
        cachedPathString.clear();
    fireGeometryChange();
}

void cPathFigure::doClearPath()
{
    for (int i = 0; i < (int)path.size(); i++)
        delete path[i];
    path.clear();
    cachedPathString.clear();
}

void cPathFigure::clearPath()
{
    doClearPath();
    fireGeometryChange();
}

void cPathFigure::addMoveTo(double x, double y)
{
    MoveTo *item = new MoveTo();
    item->code = 'M';
    item->x = x;
    item->y = y;
    addItem(item);
}

void cPathFigure::addMoveRel(double dx, double dy)
{
    MoveRel *item = new MoveRel();
    item->code = 'm';
    item->dx = dx;
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addLineTo(double x, double y)
{
    LineTo *item = new LineTo();
    item->code = 'L';
    item->x = x;
    item->y = y;
    addItem(item);
}

void cPathFigure::addLineRel(double dx, double dy)
{
    LineRel *item = new LineRel();
    item->code = 'l';
    item->dx = dx;
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addHorizontalLineTo(double x)
{
    HorizLineTo *item = new HorizLineTo();
    item->code = 'H';
    item->x = x;
    addItem(item);
}

void cPathFigure::addHorizontalLineRel(double dx)
{
    HorizLineRel *item = new HorizLineRel();
    item->code = 'h';
    item->dx = dx;
    addItem(item);
}

void cPathFigure::addVerticalLineTo(double y)
{
    VertLineTo *item = new VertLineTo();
    item->code = 'V';
    item->y = y;
    addItem(item);
}

void cPathFigure::addVerticalLineRel(double dy)
{
    VertLineRel *item = new VertLineRel();
    item->code = 'v';
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addArcTo(double rx, double ry, double phi, bool largeArc, bool sweep, double x, double y)
{
    ArcTo *item = new ArcTo();
    item->code = 'A';
    item->rx = rx;
    item->ry = ry;
    item->phi = phi;
    item->largeArc = largeArc;
    item->sweep = sweep;
    item->x = x;
    item->y = y;
    addItem(item);
}

void cPathFigure::addArcRel(double rx, double ry, double phi, bool largeArc, bool sweep, double dx, double dy)
{
    ArcRel *item = new ArcRel();
    item->code = 'a';
    item->rx = rx;
    item->ry = ry;
    item->phi = phi;
    item->largeArc = largeArc;
    item->sweep = sweep;
    item->dx = dx;
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addCurveTo(double x1, double y1, double x, double y)
{
    CurveTo *item = new CurveTo();
    item->code = 'Q';
    item->x1 = x1;
    item->y1 = y1;
    item->x = x;
    item->y = y;
    addItem(item);
}

void cPathFigure::addCurveRel(double dx1, double dy1, double dx, double dy)
{
    CurveRel *item = new CurveRel();
    item->code = 'q';
    item->dx1 = dx1;
    item->dy1 = dy1;
    item->dx = dx;
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addSmoothCurveTo(double x, double y)
{
    SmoothCurveTo *item = new SmoothCurveTo();
    item->code = 'T';
    item->x = x;
    item->y = y;
    addItem(item);
}

void cPathFigure::addSmoothCurveRel(double dx, double dy)
{
    SmoothCurveRel *item = new SmoothCurveRel();
    item->code = 't';
    item->dx = dx;
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addCubicBezierCurveTo(double x1, double y1, double x2, double y2, double x, double y)
{
    CubicBezierCurveTo *item = new CubicBezierCurveTo();
    item->code = 'C';
    item->x1 = x1;
    item->y1 = y1;
    item->x2 = x2;
    item->y2 = y2;
    item->x = x;
    item->y = y;
    addItem(item);
}

void cPathFigure::addCubicBezierCurveRel(double dx1, double dy1, double dx2, double dy2, double dx, double dy)
{
    CubicBezierCurveRel *item = new CubicBezierCurveRel();
    item->code = 'c';
    item->dx1 = dx1;
    item->dy1 = dy1;
    item->dx2 = dx2;
    item->dy2 = dy2;
    item->dx = dx;
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addSmoothCubicBezierCurveTo(double x2, double y2, double x, double y)
{
    SmoothCubicBezierCurveTo *item = new SmoothCubicBezierCurveTo();
    item->code = 'S';
    item->x2 = x2;
    item->y2 = y2;
    item->x = x;
    item->y = y;
    addItem(item);
}

void cPathFigure::addSmoothCubicBezierCurveRel(double dx2, double dy2, double dx, double dy)
{
    SmoothCubicBezierCurveRel *item = new SmoothCubicBezierCurveRel();
    item->code = 's';
    item->dx2 = dx2;
    item->dy2 = dy2;
    item->dx = dx;
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addClosePath()
{
    ClosePath *item = new ClosePath();
    item->code = 'Z';
    addItem(item);
}

void cPathFigure::move(double x, double y)
{
    setOffset(Point(getOffset()).translate(x, y));
}

void cPathFigure::setJoinStyle(JoinStyle joinStyle)
{
    if (joinStyle == this->joinStyle)
        return;
    this->joinStyle = joinStyle;
    fireVisualChange();
}

void cPathFigure::setCapStyle(CapStyle capStyle)
{
    if (capStyle == this->capStyle)
        return;
    this->capStyle = capStyle;
    fireVisualChange();
}

void cPathFigure::setFillRule(FillRule fillRule)
{
    if (fillRule == this->fillRule)
        return;
    this->fillRule = fillRule;
    fireVisualChange();
}

//----

void cAbstractTextFigure::copy(const cAbstractTextFigure& other)
{
    setPosition(other.getPosition());
    setColor(other.getColor());
    setOpacity(other.getOpacity());
    setFont(other.getFont());
    setText(other.getText());
    setAnchor(other.getAnchor());
}

cAbstractTextFigure& cAbstractTextFigure::operator=(const cAbstractTextFigure& other)
{
    if (this == &other)
        return *this;
    cFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cAbstractTextFigure::info() const
{
    std::stringstream os;
    os << "\"" << getText() << "\" at " << getPosition();
    return os.str();
}

void cAbstractTextFigure::parse(cProperty *property)
{
    cFigure::parse(property);

    const char *s;
    setPosition(parsePoint(property, PKEY_POS, 0));
    setText(opp_nulltoempty(property->getValue(PKEY_TEXT)));
    if ((s = property->getValue(PKEY_COLOR)) != nullptr)
        setColor(parseColor(s));
    if ((s = property->getValue(PKEY_OPACITY)) != nullptr)
        setOpacity(opp_atof(s));
    if (property->containsKey(PKEY_FONT))
        setFont(parseFont(property, PKEY_FONT));
    if ((s = property->getValue(PKEY_ANCHOR)) != nullptr)
        setAnchor(parseAnchor(s));
}

const char **cAbstractTextFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POS, PKEY_TEXT, PKEY_COLOR, PKEY_OPACITY, PKEY_FONT, PKEY_ANCHOR, nullptr};
        concatArrays(keys, cFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cAbstractTextFigure::move(double x, double y)
{
    position.x += x;
    position.y += y;
    fireGeometryChange();
}

void cAbstractTextFigure::setPosition(const Point& position)
{
    if (position == this->position)
        return;
    this->position = position;
    fireGeometryChange();
}

void cAbstractTextFigure::setAnchor(Anchor anchor)
{
    if (anchor == this->anchor)
        return;
    this->anchor = anchor;
    fireGeometryChange();
}

void cAbstractTextFigure::setColor(const Color& color)
{
    if (color == this->color)
        return;
    this->color = color;
    fireVisualChange();
}

void cAbstractTextFigure::setOpacity(double opacity)
{
    if (opacity == this->opacity)
        return;
    ENSURE_RANGE01(opacity);
    this->opacity = opacity;
    fireVisualChange();
}

void cAbstractTextFigure::setFont(Font font)
{
    if (font == this->font)
        return;
    this->font = font;
    fireVisualChange();
}

void cAbstractTextFigure::setText(const char* text)
{
    if (text == this->text)
        return;
    this->text = text;
    fireInputDataChange();
}

//----

cTextFigure& cTextFigure::operator=(const cTextFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractTextFigure::operator=(other);
    copy(other);
    return *this;
}

//----

cLabelFigure& cLabelFigure::operator=(const cLabelFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractTextFigure::operator=(other);
    copy(other);
    return *this;
}

//----

void cAbstractImageFigure::copy(const cAbstractImageFigure& other)
{
    setPosition(other.getPosition());
    setAnchor(other.getAnchor());
    setWidth(other.getWidth());
    setHeight(other.getHeight());
    setInterpolation(other.getInterpolation());
    setOpacity(other.getOpacity());
    setTintColor(other.getTintColor());
    setTintAmount(other.getTintAmount());
}

cAbstractImageFigure& cAbstractImageFigure::operator=(const cAbstractImageFigure& other)
{
    if (this == &other)
        return *this;
    cFigure::operator=(other);
    copy(other);
    return *this;
}

void cAbstractImageFigure::parse(cProperty *property)
{
    cFigure::parse(property);

    const char *s;
    setPosition(parsePoint(property, PKEY_POS, 0));
    if ((s = property->getValue(PKEY_ANCHOR)) != nullptr)
        setAnchor(parseAnchor(s));
    Point size = parsePoint(property, PKEY_SIZE, 0);
    setWidth(size.x);
    setHeight(size.y);
    if ((s = property->getValue(PKEY_INTERPOLATION)) != nullptr)
        setInterpolation(parseInterpolation(s));
    if ((s = property->getValue(PKEY_OPACITY)) != nullptr)
        setOpacity(opp_atof(s));
    if ((s = property->getValue(PKEY_TINT, 0)) != nullptr) {
        setTintColor(parseColor(s));
        setTintAmount(0.5);
    }
    if ((s = property->getValue(PKEY_TINT, 1)) != nullptr)
        setTintAmount(opp_atof(s));
}

const char **cAbstractImageFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POS, PKEY_ANCHOR, PKEY_SIZE, PKEY_INTERPOLATION, PKEY_OPACITY, PKEY_TINT, nullptr};
        concatArrays(keys, cFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cAbstractImageFigure::move(double x, double y)
{
    position.x += x;
    position.y += y;
    fireGeometryChange();
}


void cAbstractImageFigure::setPosition(const Point& position)
{
    if (position == this->position)
        return;
    this->position = position;
    fireGeometryChange();
}

void cAbstractImageFigure::setAnchor(Anchor anchor)
{
    if (anchor == this->anchor)
        return;
    this->anchor = anchor;
    fireGeometryChange();
}

void cAbstractImageFigure::setWidth(double width)
{
    if (width == this->width)
        return;
    ENSURE_NONNEGATIVE(width);
    this->width = width;
    fireGeometryChange();
}

void cAbstractImageFigure::setHeight(double height)
{
    if (height == this->height)
        return;
    ENSURE_NONNEGATIVE(height);
    this->height = height;
    fireGeometryChange();
}

void cAbstractImageFigure::setInterpolation(Interpolation interpolation)
{
    if (interpolation == this->interpolation)
        return;
    this->interpolation = interpolation;
    fireVisualChange();
}

void cAbstractImageFigure::setOpacity(double opacity)
{
    if (opacity == this->opacity)
        return;
    ENSURE_RANGE01(opacity);
    this->opacity = opacity;
    fireVisualChange();
}

void cAbstractImageFigure::setTintColor(const Color& tintColor)
{
    if (tintColor == this->tintColor)
        return;
    this->tintColor = tintColor;
    fireVisualChange();
}

void cAbstractImageFigure::setTintAmount(double tintAmount)
{
    if (tintAmount == this->tintAmount)
        return;
    ENSURE_RANGE01(tintAmount);
    this->tintAmount = tintAmount;
    fireVisualChange();
}

//----

std::string cImageFigure::info() const
{
    std::stringstream os;
    os << "\"" << getImageName() << "\" at " << getPosition();
    return os.str();
}

cImageFigure& cImageFigure::operator=(const cImageFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractImageFigure::operator=(other);
    copy(other);
    return *this;
}

void cImageFigure::copy(const cImageFigure& other)
{
    setImageName(other.getImageName());
}

void cImageFigure::parse(cProperty *property)
{
    setImageName(opp_nulltoempty(property->getValue(PKEY_IMAGE)));
    cAbstractImageFigure::parse(property);
}

const char **cImageFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_IMAGE, nullptr};
        concatArrays(keys, cAbstractImageFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cImageFigure::setImageName(const char* imageName)
{
    if (imageName == this->imageName)
        return;
    this->imageName = imageName;
    fireInputDataChange();
}

//----

cIconFigure& cIconFigure::operator=(const cIconFigure& other)
{
    if (this == &other) return *this;
    cImageFigure::operator=(other);
    copy(other);
    return *this;
}

//----

std::string cPixmapFigure::info() const
{
    std::stringstream os;
    os << "(" << getPixmapWidth() << " x " << getPixmapHeight() << ") at " << getPosition();
    return os.str();
}

cPixmapFigure& cPixmapFigure::operator=(const cPixmapFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractImageFigure::operator=(other);
    copy(other);
    return *this;
}

void cPixmapFigure::copy(const cPixmapFigure& other)
{
    setPixmap(other.getPixmap());
}

void cPixmapFigure::parse(cProperty *property)
{
    cAbstractImageFigure::parse(property);

    int width = 0, height = 0;
    const char *s;
    if ((s = property->getValue(PKEY_RESOLUTION, 0)) != nullptr)
        width = opp_atoul(s);
    if ((s = property->getValue(PKEY_RESOLUTION, 1)) != nullptr)
        height = opp_atoul(s);
    if (width > 0 || height > 0)
        setPixmap(Pixmap(width, height));
}

const char **cPixmapFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_RESOLUTION, nullptr };
        concatArrays(keys, cAbstractImageFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cPixmapFigure::setPixmap(const Pixmap& pixmap)
{
    if (&pixmap != &this->pixmap) // don't copy if it's the same object
        this->pixmap = pixmap;
    fireInputDataChange(); // always notify, as content may have changed
}

void cPixmapFigure::resize(int width, int height, const RGBA& fill)
{
    pixmap.resize(width, height, fill);
    fireInputDataChange();
}

void cPixmapFigure::resize(int width, int height, const Color& color, double opacity)
{
    if (pixmap.getWidth() == width && pixmap.getHeight() == height)
        return;
    ENSURE_RANGE01(opacity);
    pixmap.resize(width, height, color, opacity);
    fireInputDataChange();
}

void cPixmapFigure::fill(const RGBA& fill)
{
    pixmap.fill(fill);
    fireInputDataChange();
}

void cPixmapFigure::fill(const Color& color, double opacity)
{
    ENSURE_RANGE01(opacity);
    pixmap.fill(color, opacity);
    fireInputDataChange();
}

void cPixmapFigure::setPixel(int x, int y, const RGBA& argb)
{
    pixmap.pixel(x, y) = argb;
    fireInputDataChange();
}

void cPixmapFigure::setPixel(int x, int y, const Color& color, double opacity)
{
    ENSURE_RANGE01(opacity);
    pixmap.setPixel(x, y, color, opacity);
    fireInputDataChange();
}

void cPixmapFigure::setPixelColor(int x, int y, const Color& color)
{
    pixmap.setColor(x, y, color);
    fireInputDataChange();
}

void cPixmapFigure::setPixelOpacity(int x, int y, double opacity)
{
    ENSURE_RANGE01(opacity);
    pixmap.setOpacity(x, y, opacity);
    fireInputDataChange();
}

//------

cCanvas::cCanvas(const char *name) : cOwnedObject(name), backgroundColor(cFigure::Color(160,224,160))
{
    rootFigure = new cGroupFigure("rootFigure");
    take(rootFigure);
}

cCanvas::~cCanvas()
{
    dropAndDelete(rootFigure);
}

void cCanvas::copy(const cCanvas& other)
{
    setBackgroundColor(other.getBackgroundColor());
    dropAndDelete(rootFigure);
    tagBitIndex.clear();
    rootFigure = other.getRootFigure()->dupTree();
    take(rootFigure);
}

cCanvas& cCanvas::operator=(const cCanvas& other)
{
    if (this == &other)
        return *this;
    cOwnedObject::operator=(other);
    copy(other);
    return *this;
}

void cCanvas::forEachChild(cVisitor *v)
{
    rootFigure->forEachChild(v);  // skip the root figure from the tree
}

std::string cCanvas::info() const
{
    std::stringstream os;
    os << rootFigure->getNumFigures() << " toplevel figure(s)";
    return os.str();
}

bool cCanvas::containsCanvasItems(cProperties *properties)
{
    for (int i = 0; i < properties->getNumProperties(); i++) {
        cProperty *property = properties->get(i);
        if (property->isName("figure"))
            return true;
    }
    return false;
}

void cCanvas::addFiguresFrom(cProperties *properties)
{
    std::map<cFigure *, double> orderMap;

    // Note: the following code assumes that parent figures precede their children, otherwise a "parent not found" error will occur
    for (int i = 0; i < properties->getNumProperties(); i++) {
        cProperty *property = properties->get(i);
        if (property->isName("figure"))
            parseFigure(property, orderMap);
    }
}

void cCanvas::parseFigure(cProperty *property, std::map<cFigure *, double>& orderMap) const
{
    try {
        const char *path = property->getIndex();
        if (!path)
            throw cRuntimeError("@figure property is expected to have an index which will become the figure name, e.g. @figure[foo]");
        const char *lastDot = strrchr(path, '.');
        cFigure *parent;
        const char *name;
        if (lastDot) {
            std::string parentPath = std::string(path, lastDot - path);
            parent = getFigureByPath(parentPath.c_str());
            if (!parent)
                throw cRuntimeError("parent figure \"%s\" not found", parentPath.c_str());
            name = lastDot + 1;
        }
        else {
            parent = rootFigure;
            name = path;
        }
        if (!name[0])
            throw cRuntimeError("figure name cannot be empty");

        cFigure *figure = parent->getFigure(name);
        if (!figure) {
            const char *type = opp_nulltoempty(property->getValue(PKEY_TYPE));
            figure = createFigure(type);
            figure->setName(name);
            const char *order = property->getValue(PKEY_CHILDZ);
            if (order)
                orderMap[figure] = opp_atof(order);
            parent->insertChild(figure, orderMap);
        }
        else {
            figure->raiseToTop();
        }

        figure->parse(property);
    }
    catch (std::exception& e) {
        throw cRuntimeError(this, "Error creating figure from NED property @%s: %s", property->getFullName(), e.what());
    }
}

cFigure *cCanvas::createFigure(const char *type) const
{
    cFigure *figure;
    if (!strcmp(type, "group"))
        figure = new cGroupFigure();
//    else if (!strcmp(type, "panel"))
//        figure = new cPanelFigure();
    else if (!strcmp(type, "line"))
        figure = new cLineFigure();
    else if (!strcmp(type, "arc"))
        figure = new cArcFigure();
    else if (!strcmp(type, "polyline"))
        figure = new cPolylineFigure();
    else if (!strcmp(type, "rectangle"))
        figure = new cRectangleFigure();
    else if (!strcmp(type, "oval"))
        figure = new cOvalFigure();
    else if (!strcmp(type, "ring"))
        figure = new cRingFigure();
    else if (!strcmp(type, "pieslice"))
        figure = new cPieSliceFigure();
    else if (!strcmp(type, "polygon"))
        figure = new cPolygonFigure();
    else if (!strcmp(type, "path"))
        figure = new cPathFigure();
    else if (!strcmp(type, "text"))
        figure = new cTextFigure();
    else if (!strcmp(type, "label"))
        figure = new cLabelFigure();
    else if (!strcmp(type, "image"))
        figure = new cImageFigure();
    else if (!strcmp(type, "icon"))
        figure = new cIconFigure();
    else if (!strcmp(type, "pixmap"))
        figure = new cPixmapFigure();
    else {
        // find registered class named "<type>Figure" or "c<type>Figure"
        std::string className = std::string("c") + type + "Figure";
        className[1] = opp_toupper(className[1]);
        cObjectFactory *factory = cObjectFactory::find(className.c_str() + 1);  // type without leading "c"
        if (!factory)
            factory = cObjectFactory::find(className.c_str());  // try with leading "c"
        if (!factory)
            throw cRuntimeError("No figure class registered with name '%s' or '%s'", className.c_str() + 1, className.c_str());
        cObject *obj = factory->createOne();
        figure = dynamic_cast<cFigure *>(obj);
        if (!figure)
            throw cRuntimeError("Wrong figure class: cannot cast %s to cFigure", obj->getClassName());
    }
    return figure;
}

void cCanvas::dumpSupportedPropertyKeys(std::ostream& out) const
{
    const char *types[] = {
        "group", /*"panel",*/ "line", "arc", "polyline", "rectangle", "oval", "ring", "pieslice",
        "polygon", "path", "text", "label", "image", "pixmap", nullptr
    };

    for (const char **p = types; *p; p++) {
        const char *type = *p;
        cFigure *figure = createFigure(type);
        out << type << ": " << opp_join(figure->getAllowedPropertyKeys(), ", ") << "\n";
        delete figure;
    }
}

cFigure *cCanvas::getSubmodulesLayer() const
{
    return rootFigure->getFigure("submodules");
}

uint64_t cCanvas::parseTags(const char *s)
{
    uint64_t result = 0;
    cStringTokenizer tokenizer(s);
    while (tokenizer.hasMoreTokens()) {
        const char *tag = tokenizer.nextToken();
        int bitIndex;
        std::map<std::string, int>::const_iterator it = tagBitIndex.find(tag);
        if (it != tagBitIndex.end())
            bitIndex = it->second;
        else {
            bitIndex = tagBitIndex.size();
            if (bitIndex >= 64)
                throw cRuntimeError(this, "Cannot add figure tag \"%s\": maximum 64 tags supported", tag);
            tagBitIndex[tag] = bitIndex;
        }
        result |= ((uint64_t)1) << bitIndex;
    }
    return result;
}

std::string cCanvas::getTags(uint64_t tagBits)
{
    std::stringstream os;
    for (std::map<std::string, int>::const_iterator it = tagBitIndex.begin(); it != tagBitIndex.end(); ++it) {
        if ((tagBits & (((uint64_t)1) << it->second)) != 0) {
            if (it != tagBitIndex.begin())
                os << " ";
            os << it->first;
        }
    }
    return os.str();
}

std::string cCanvas::getAllTags() const
{
    std::stringstream os;
    for (std::map<std::string, int>::const_iterator it = tagBitIndex.begin(); it != tagBitIndex.end(); ++it) {
        if (it != tagBitIndex.begin())
            os << " ";
        os << it->first;
    }
    return os.str();
}

std::vector<std::string> cCanvas::getAllTagsAsVector() const
{
    std::vector<std::string> result;
    for (std::map<std::string, int>::const_iterator it = tagBitIndex.begin(); it != tagBitIndex.end(); ++it)
        result.push_back(it->first);
    return result;
}

}  // namespace omnetpp

