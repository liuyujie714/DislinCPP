#pragma once

/**
 * ╔══════════════════════════════════════════════════════════════════════════╗
 * ║              DislinPlot.h  —  matplotlib-style C++ wrapper               ║
 * ║                         around the DISLIN library                        ║
 * ╚══════════════════════════════════════════════════════════════════════════╝
 *
 * Requires:  discpp.h on the include path
 * Link with: -ldislin -lX11 -lm   (Linux)
 *            discpp.lib           (Windows MSVC)
 *
 * ── Quick start ────
 *
 *   DislinPlot plt;
 *   plt.figure("My Window");       // title shown in the window title bar
 *   plt.plot(x, y, "red");         // add a red line curve
 *   plt.xlabel("Time (s)");
 *   plt.ylabel("Amplitude");
 *   plt.title("My Signal");
 *   plt.grid("on");
 *   plt.show();                    // render to screen
 *   // plt.savefig("out.png");     // or save to file
 *
 * ── figure() ──────
 *
 *   plt.figure(win_title, output, page)
 *     win_title : string shown in the window title bar
 *     output    : "cons" (screen, default) | "png" | "pdf" | "eps" | "svg" | "gif"
 *     page      : "da4l" (A4 landscape, default) | "da4p" (A4 portrait) | "da3l" …
 *
 * ── Chart types ────
 *
 *   plt.plot(x, y, color, label)
 *       Line / curve chart. x and y are std::vector<double> or (double*, int n).
 *
 *   plt.scatter(x, y, color, symbol, label)
 *       Scatter plot. symbol is a DISLIN symbol index (default 21 = filled circle).
 *       Common symbols: 0 dot · 1 plus · 3 circle · 4 cross · 21 filled circle
 *
 *   plt.bar(x, y, color, label)
 *       Vertical bar chart. Bars are drawn from 0 to y[i].
 *
 *   plt.bar_grouped(cats, data, colors, labels)
 *       Side-by-side grouped bars.
 *       cats        : x-positions of groups  (std::vector<double>)
 *       data        : data[group][category]  (std::vector<std::vector<double>>)
 *       colors/labels : one entry per group
 *
 *   plt.pie(values, sliceLabels)
 *       Pie chart. values need not sum to 1; DISLIN normalises automatically.
 *
 *   plt.hist(data, bins, color, label)
 *       Histogram. Bins the raw data automatically into `bins` equal-width bars.
 *
 * ── Appearance ─────
 *
 *   plt.title("Chart Title")       // main chart title
 *   plt.xlabel("X label")          // x-axis label
 *   plt.ylabel("Y label")          // y-axis label
 *   plt.grid("on")                 // "on" | "off"  (default "off")
 *   plt.legend()                   // show legend  (auto-shown if any label is set)
 *   plt.axes_bg(r, g, b)           // axes background colour, RGB in [0, 1]
 *   plt.symbol_size(sz)            // scatter symbol size in plot units (default 45)
 *   plt.title_gap(offset)          // vertical distance between title and axis box
 *                                  //   negative → title moves up (more space)
 *                                  //   positive → title moves down (less space)
 *                                  //   units: 1/100 cm,  e.g. -300 = -3 cm
 *
 * ── Colors ─────
 *
 *   DISLIN named colors : "red" "green" "blue" "yellow" "cyan" "white" "fore"
 *   Extended names      : "steelblue" "orange" "purple" "brown" "pink" "gray"
 *   Custom RGB          : "rgb:0.2,0.6,0.9"   (each channel in [0, 1])
 *
 * ── Axis limits ────
 *
 *   plt.xlim(lo, hi)               // fix x-axis range manually
 *   plt.ylim(lo, hi)               // fix y-axis range manually
 *   (if not called, ranges are computed automatically from the data)
 *
 * ── Text annotations ───
 *
 *   plt.text("label", x, y)        // draw text at data coordinates (x, y)
 *   plt.text("label", x, y, "red") // with a specific color
 *
 * ── Output ──
 *
 *   plt.show()                     // display on screen (blocks until window is closed)
 *   plt.savefig("chart.png")       // save as PNG  (extension sets the format)
 *   plt.savefig("chart.pdf")       // save as PDF
 *   plt.savefig("chart.svg")       // save as SVG
 *   plt.savefig("chart.eps")       // save as EPS
 *
 * ── Subplots ───
 *
 *   All panel data is collected first, then rendered in a SINGLE Dislin
 *   session when show() / savefig() is called.
 *
 *   plt.figure("2x2 grid");
 *   plt.subplot_layout(2, 2);      // must be called BEFORE any subplot()
 *
 *   plt.subplot(0, 0);             // top-left  panel  (row, col, 0-based)
 *   plt.title("sin");
 *   plt.plot(x, y_sin, "red");
 *   plt.grid("on");
 *
 *   plt.subplot(0, 1);             // top-right panel
 *   plt.title("cos");
 *   plt.plot(x, y_cos, "blue");
 *
 *   plt.subplot(1, 0);             // bottom-left panel
 *   plt.subplot(1, 1);             // bottom-right panel
 *   // … add plots as usual …
 *
 *   plt.show();                    // draws all 4 panels in one window
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "discpp.h"

// ==============================================================
namespace dp_detail
{

inline double ceilTo(double v, double s)
{
    return std::ceil(v / s) * s;
}
inline double floorTo(double v, double s)
{
    return std::floor(v / s) * s;
}

inline double niceStep(double range)
{
    if (range <= 0) return 1.0;
    double raw  = range / 5.0;
    double mag  = std::pow(10.0, std::floor(std::log10(raw)));
    double norm = raw / mag;
    return ((norm < 1.5) ? 1.0 : (norm < 3.5) ? 2.0 : (norm < 7.5) ? 5.0 : 10.0) * mag;
}

// Axis range that strictly contains [dlo,dhi]
inline void niceAxis(double dlo, double dhi, double& axMin, double& axMax, double& step)
{
    if (dlo == dhi)
    {
        dlo -= 1.0;
        dhi += 1.0;
    }
    step  = niceStep(dhi - dlo);
    axMin = floorTo(dlo, step);
    axMax = ceilTo(dhi, step);
    if (axMin >= dlo) axMin -= step;
    if (axMax <= dhi) axMax += step;
    if ((axMax - axMin) < 2 * step) axMax = axMin + 2 * step;
}
} // namespace dp_detail

// ==============================================================
struct PlotStyle
{
    std::string color  = "red";
    std::string label  = "";
    int         symbol = 21;
};

// ==============================================================
class DislinPlot
{
public:
    DislinPlot()  = default;
    ~DislinPlot() = default;

    // ==============================================================
    void figure(const std::string& win_title = "DislinPlot",
                const std::string& output    = "cons",
                const std::string& page      = "da4l")
    {
        reset_state();
        win_title_ = win_title;
        output_    = output;
        page_      = page;
    }

    void xlabel(const std::string& s) { cur().xlabelStr = s; }
    void ylabel(const std::string& s) { cur().ylabelStr = s; }
    void title(const std::string& s) { cur().title = s; }
    void title_gap(int offset) { cur().titleGap = offset; }
    void grid(const std::string& s = "on") { cur().gridMode = s; }
    void legend() { cur().showLegend = true; }
    void symbol_size(int sz) { symbolSz_ = sz; }
    void axes_bg(double r, double g, double b)
    {
        cur().axBgR   = r;
        cur().axBgG   = g;
        cur().axBgB   = b;
        cur().useAxBg = true;
    }
    void xlim(double lo, double hi)
    {
        cur().manualXmin = lo;
        cur().manualXmax = hi;
        cur().manualX    = true;
    }
    void ylim(double lo, double hi)
    {
        cur().manualYmin = lo;
        cur().manualYmax = hi;
        cur().manualY    = true;
    }

    // ==============================================================
    void plot(const std::vector<double>& x,
              const std::vector<double>& y,
              const std::string&         color = "red",
              const std::string&         label = "")
    {
        if (x.size() != y.size() || x.empty()) throw std::runtime_error("plot: x/y size mismatch");
        Series s;
        s.kind        = "line";
        s.x           = x;
        s.y           = y;
        s.style.color = color;
        s.style.label = label;
        if (!label.empty()) cur().showLegend = true;
        cur().series.push_back(std::move(s));
    }
    void plot(const double*      x,
              const double*      y,
              int                n,
              const std::string& color = "red",
              const std::string& label = "")
    {
        plot({x, x + n}, {y, y + n}, color, label);
    }

    void scatter(const std::vector<double>& x,
                 const std::vector<double>& y,
                 const std::string&         color  = "blue",
                 int                        symbol = 21,
                 const std::string&         label  = "")
    {
        if (x.size() != y.size() || x.empty())
            throw std::runtime_error("scatter: x/y size mismatch");
        Series s;
        s.kind         = "scatter";
        s.x            = x;
        s.y            = y;
        s.style.color  = color;
        s.style.symbol = symbol;
        s.style.label  = label;
        if (!label.empty()) cur().showLegend = true;
        cur().series.push_back(std::move(s));
    }
    void scatter(const double*      x,
                 const double*      y,
                 int                n,
                 const std::string& color  = "blue",
                 int                symbol = 21,
                 const std::string& label  = "")
    {
        scatter({x, x + n}, {y, y + n}, color, symbol, label);
    }

    void bar(const std::vector<double>& x,
             const std::vector<double>& y,
             const std::string&         color = "steelblue",
             const std::string&         label = "")
    {
        if (x.size() != y.size() || x.empty()) throw std::runtime_error("bar: x/y size mismatch");
        Series s;
        s.kind        = "bar";
        s.x           = x;
        s.y           = y;
        s.y0          = std::vector<double>(y.size(), 0.0);
        s.style.color = color;
        s.style.label = label;
        if (!label.empty()) cur().showLegend = true;
        cur().series.push_back(std::move(s));
    }
    void bar(const double*      x,
             const double*      y,
             int                n,
             const std::string& color = "steelblue",
             const std::string& label = "")
    {
        bar({x, x + n}, {y, y + n}, color, label);
    }

    void bar_grouped(const std::vector<double>&              cats,
                     const std::vector<std::vector<double>>& data,
                     const std::vector<std::string>&         colors = {},
                     const std::vector<std::string>&         labels = {})
    {
        static const char* dc[] = {"red", "blue", "green", "yellow", "cyan", "orange", "fore"};
        int                ng   = (int)data.size();
        for (int g = 0; g < ng; g++)
        {
            Series s;
            s.kind        = "bar";
            s.x           = cats;
            s.y           = data[g];
            s.y0          = std::vector<double>(cats.size(), 0.0);
            s.style.color = (g < (int)colors.size()) ? colors[g] : dc[g % 7];
            s.style.label = (g < (int)labels.size()) ? labels[g] : "";
            s.groupIndex  = g;
            s.groupCount  = ng;
            if (!s.style.label.empty()) cur().showLegend = true;
            cur().series.push_back(std::move(s));
        }
    }

    void pie(const std::vector<double>& values, const std::vector<std::string>& sliceLabels = {})
    {
        Series s;
        s.kind      = "pie";
        s.y         = values;
        s.pieLabels = sliceLabels;
        cur().series.push_back(std::move(s));
    }

    void hist(const std::vector<double>& data,
              int                        bins  = 10,
              const std::string&         color = "steelblue",
              const std::string&         label = "")
    {
        if (data.empty()) return;
        double lo = *std::min_element(data.begin(), data.end());
        double hi = *std::max_element(data.begin(), data.end());
        if (lo == hi)
        {
            lo -= 1;
            hi += 1;
        }
        double              bw = (hi - lo) / bins;
        std::vector<double> xc(bins), yc(bins, 0.0);
        for (int b = 0; b < bins; b++)
            xc[b] = lo + (b + 0.5) * bw;
        for (double v : data)
        {
            int b = (int)((v - lo) / bw);
            if (b == bins) b--;
            if (b >= 0 && b < bins) yc[b]++;
        }
        bar(xc, yc, color, label);
    }

    void text(const std::string& msg, double x, double y, const std::string& color = "fore")
    {
        Annotation a;
        a.msg   = msg;
        a.x     = x;
        a.y     = y;
        a.color = color;
        cur().annotations.push_back(std::move(a));
    }

    // ==============================================================
    /**
     * subplot_layout(rows, cols): call once before subplot().
     * subplot(row, col)         : switch to that panel (0-based).
     *   — Each subplot() commits the current panel and starts a new one.
     *   — show()/savefig() draws ALL panels in one Dislin session.
     */
    void subplot_layout(int nrows, int ncols)
    {
        subRows_    = nrows;
        subCols_    = ncols;
        useSubplot_ = true;
        // Initialise panel list with one empty panel at (0,0)
        panels_.clear();
        Panel p;
        p.row = 0;
        p.col = 0;
        panels_.push_back(std::move(p));
        curPanel_ = 0;
    }

    void subplot(int row, int col)
    {
        // Commit the current panel (already in panels_), start a new one
        Panel p;
        p.row = subRows_ - 1 - row; // sub figure from top to bottom
        p.col = col;
        panels_.push_back(std::move(p));
        curPanel_ = (int)panels_.size() - 1;
    }

    // ==============================================================
    void show() { render_all("cons", ""); }
    void savefig(const std::string& path)
    {
        std::string ext;
        auto        dot = path.rfind('.');
        if (dot != std::string::npos) ext = path.substr(dot + 1);
        std::transform(ext.begin(),
                       ext.end(),
                       ext.begin(),
                       [](char c) { return (char)std::tolower((unsigned char)c); });
        const std::vector<std::string> supported_fmts = {"png", "pdf", "eps", "svg", "gif", "tiff"};
        std::string                    fmt            = "png";
        if (std::find(supported_fmts.cbegin(), supported_fmts.cend(), ext) != supported_fmts.cend())
        {
            fmt = ext;
        }
        render_all(fmt, path);
    }

private:
    // ==============================================================
    struct Series
    {
        std::string              kind;
        std::vector<double>      x, y, y0;
        PlotStyle                style;
        int                      groupIndex = 0, groupCount = 1;
        std::vector<std::string> pieLabels;
    };
    struct Annotation
    {
        std::string msg, color;
        double      x = 0, y = 0;
    };

    // One panel = one set of series + its own config
    struct Panel
    {
        int                     row = 0, col = 0, titleGap = -100;
        std::string             title, xlabelStr, ylabelStr, gridMode = "off", panelTitle;
        bool                    manualX = false, manualY = false;
        double                  manualXmin = 0, manualXmax = 1, manualYmin = 0, manualYmax = 1;
        bool                    useAxBg = false;
        double                  axBgR = 0.97, axBgG = 0.97, axBgB = 0.97;
        bool                    showLegend = false;
        std::vector<Series>     series;
        std::vector<Annotation> annotations;
    };

    std::string             win_title_ = "DislinPlot", output_ = "cons", page_ = "da4l";
    int                     symbolSz_   = 45;
    bool                    useSubplot_ = false;
    int                     subRows_ = 1, subCols_ = 1, curPanel_ = 0;
    std::vector<Panel>      panels_;
    std::unique_ptr<Dislin> g_;
    std::vector<char>       legBuf_;

    // ==============================================================
    Panel& cur()
    {
        if (panels_.empty())
        {
            Panel p;
            panels_.push_back(std::move(p));
            curPanel_ = 0;
        }
        return panels_[curPanel_];
    }

    void reset_state()
    {
        panels_.clear();
        curPanel_   = 0;
        win_title_  = "DislinPlot";
        output_     = "cons";
        page_       = "da4l";
        useSubplot_ = false;
        subRows_ = subCols_ = 1;
        symbolSz_           = 45;
        g_.reset();
        legBuf_.clear();
        // always have at least one panel
        panels_.push_back(Panel{});
    }

    // ==============================================================
    void applyColor(const std::string& col)
    {
        if (col.size() > 4 && col.substr(0, 4) == "rgb:")
        {
            double r, gg, b;
            std::sscanf(col.c_str() + 4, "%lf,%lf,%lf", &r, &gg, &b);
            g_->setrgb(r, gg, b);
        }
        else if (col == "steelblue") { g_->setrgb(0.27, 0.51, 0.71); }
        else if (col == "orange") { g_->setrgb(1.0, 0.50, 0.0); }
        else if (col == "purple") { g_->setrgb(0.5, 0.0, 0.5); }
        else if (col == "brown") { g_->setrgb(0.6, 0.3, 0.1); }
        else if (col == "pink") { g_->setrgb(1.0, 0.41, 0.71); }
        else if (col == "gray" || col == "grey") { g_->setrgb(0.5, 0.5, 0.5); }
        else { g_->color(col.c_str()); }
    }

    // ==============================================================
    bool xyBounds(const Panel& p, double& xlo, double& xhi, double& ylo, double& yhi) const
    {
        xlo = ylo = std::numeric_limits<double>::infinity();
        xhi = yhi = -std::numeric_limits<double>::infinity();
        for (auto& s : p.series)
        {
            if (s.kind == "pie") continue;
            for (double v : s.x)
            {
                xlo = std::min(xlo, v);
                xhi = std::max(xhi, v);
            }
            for (double v : s.y)
            {
                ylo = std::min(ylo, v);
                yhi = std::max(yhi, v);
            }
            for (double v : s.y0)
            {
                ylo = std::min(ylo, v);
            }
        }
        return std::isfinite(xlo);
    }

    // ==============================================================
    // DISLIN A4L page = 2970 × 2100 internal units
    // axspos(x,y) = BOTTOM-LEFT corner of axis box
    // axslen(w,h) = width and height of axis box
    struct PanelGeom
    {
        int x, y, w, h;
    };
    PanelGeom panelGeom(int row, int col) const
    {
        // Leave generous margins so tick labels / titles don't overlap
        const int pageW = 2970, pageH = 2100;
        const int marginL = 350, marginR = 100, marginT = 200, marginB = 250;
        const int gapX = 250, gapY = 350; // gaps between panels

        int totalW = pageW - marginL - marginR;
        int totalH = pageH - marginT - marginB;
        // Each cell width/height (including one gap)
        int cellW = (totalW - (subCols_ - 1) * gapX) / subCols_;
        int cellH = (totalH - (subRows_ - 1) * gapY) / subRows_;

        PanelGeom pg;
        pg.w = cellW;
        pg.h = cellH;
        // x: left edge of axis box
        pg.x = marginL + col * (cellW + gapX);
        // y: BOTTOM edge of axis box (DISLIN counts from bottom)
        // row 0 is the TOP row → largest y value
        pg.y = marginB + (subRows_ - 1 - row) * (cellH + gapY) + cellH;
        return pg;
    }

    // ==============================================================
    void buildLegend(int nlin, int maxlen = 20)
    {
        legBuf_.assign((maxlen + 1) * nlin + 64, 0);
        g_->legini(legBuf_.data(), nlin, maxlen);
    }

    // ==============================================================
    void drawXYPanel(const Panel& p, int axX, int axY, int axW, int axH)
    {
        // 1. axis ranges
        double dxlo, dxhi, dylo, dyhi;
        if (!xyBounds(p, dxlo, dxhi, dylo, dyhi))
        {
            dxlo = 0;
            dxhi = 1;
            dylo = 0;
            dyhi = 1;
        }
        for (auto& s : p.series)
            if (s.kind == "bar") dylo = std::min(dylo, 0.0);

        double axXmin, axXmax, xstep, axYmin, axYmax, ystep;
        if (p.manualX)
        {
            axXmin = p.manualXmin;
            axXmax = p.manualXmax;
            xstep  = dp_detail::niceStep(axXmax - axXmin);
        }
        else { dp_detail::niceAxis(dxlo, dxhi, axXmin, axXmax, xstep); }

        if (p.manualY)
        {
            axYmin = p.manualYmin;
            axYmax = p.manualYmax;
            ystep  = dp_detail::niceStep(axYmax - axYmin);
        }
        else { dp_detail::niceAxis(dylo, dyhi, axYmin, axYmax, ystep); }

        // 2. position & axis
        g_->axspos(axX, axY);
        g_->axslen(axW, axH);
        if (!p.xlabelStr.empty()) g_->name(p.xlabelStr.c_str(), "x");
        if (!p.ylabelStr.empty()) g_->name(p.ylabelStr.c_str(), "y");
        if (p.useAxBg)
        {
            int ic = g_->intrgb(p.axBgR, p.axBgG, p.axBgB);
            g_->axsbgd(ic);
        }
        g_->graf(axXmin, axXmax, axXmin, xstep, axYmin, axYmax, axYmin, ystep);

        // 3. grid
        if (p.gridMode == "on")
        {
            g_->setrgb(0.75, 0.75, 0.75);
            g_->grid(1, 1);
            g_->color("fore");
        }

        // 4. legend init
        int nleg = 0;
        if (p.showLegend)
            for (auto& s : p.series)
                if (!s.style.label.empty()) nleg++;
        if (nleg > 0) buildLegend(nleg);
        int legIdx = 1;

        // 5. grouped bar prefix
        int maxGrp = 1;
        for (auto& s : p.series)
            if (s.kind == "bar") maxGrp = std::max(maxGrp, s.groupCount);
        if (maxGrp > 1) g_->bargrp(maxGrp, 0.15);

        // 6. draw series
        for (auto& s : p.series)
        {
            if (s.kind == "pie") continue;
            applyColor(s.style.color);
            if (s.kind == "line") { g_->curve(s.x.data(), s.y.data(), (int)s.x.size()); }
            else if (s.kind == "scatter")
            {
                g_->hsymbl(symbolSz_);
                g_->incmrk(-1);
                g_->marker(s.style.symbol);
                g_->curve(s.x.data(), s.y.data(), (int)s.x.size());
                g_->incmrk(0);
            }
            else if (s.kind == "bar")
            {
                g_->bars(const_cast<double*>(s.x.data()),
                         const_cast<double*>(s.y0.data()),
                         const_cast<double*>(s.y.data()),
                         (int)s.x.size());
            }
            if (nleg > 0 && !s.style.label.empty())
                g_->leglin(legBuf_.data(), s.style.label.c_str(), legIdx++);
        }
        if (maxGrp > 1) g_->reset("bargrp");

        // 7. annotations
        for (auto& a : p.annotations)
        {
            double ax = std::max(axXmin, std::min(axXmax, a.x));
            double ay = std::max(axYmin, std::min(axYmax, a.y));
            applyColor(a.color);
            g_->messag(a.msg.c_str(), g_->nxposn(ax), g_->nyposn(ay));
        }
        g_->color("fore");

        // 8. legend
        if (nleg > 0)
        {
            g_->legtit(" ");
            g_->legend(legBuf_.data(), 7);
        }

        // 9. title
        if (!p.title.empty())
        {
            g_->height(40);
            g_->titlin(p.title.c_str(), 2);
            g_->vkytit(p.titleGap);
            g_->title();
        }
    }

    // ==============================================================
    void drawPiePanel(const Panel& p)
    {
        for (auto& s : p.series)
        {
            if (s.kind != "pie") continue;
            int n = (int)s.y.size();
            if (!n) continue;
            g_->axslen(1600, 1600);
            g_->axspos(650, 1950);
            int nleg = (int)s.pieLabels.size();
            if (nleg > 0)
            {
                buildLegend(nleg, 16);
                int li = 1;
                for (auto& lb : s.pieLabels)
                    g_->leglin(legBuf_.data(), lb.c_str(), li++);
            }
            if (nleg > 0)
            {
                g_->labels("data", "pie");
                g_->labpos("external", "pie");
                g_->chnpie("both");
            }
            g_->piegrf(legBuf_.data(), nleg > 0 ? 1 : 0, s.y.data(), n);
            if (!p.title.empty())
            {
                g_->height(50);
                g_->titlin(p.title.c_str(), 4);
                g_->title();
            }
        }
    }

    // ==============================================================
    void render_all(const std::string& fmt, const std::string& filepath)
    {
        g_ = std::make_unique<Dislin>();
        g_->metafl(fmt.c_str());
        g_->scrmod("revers");
        g_->setpag(page_.c_str());
        if (!filepath.empty())
        {
            g_->filmod("delete");
            g_->setfil(filepath.c_str());
        }
        g_->disini();
        g_->errmod("ALL", "OFF");
        g_->hwfont();
        if (fmt == "cons") { g_->wintit(win_title_.c_str()); }
        // g_->helves();
        // g_->shdcha();

        if (!useSubplot_)
        {
            // Single panel: always panels_[0]
            Panel& p      = panels_[0];
            bool   hasPie = false;
            for (auto& s : p.series)
            {
                if (s.kind == "pie")
                {
                    hasPie = true;
                    break;
                }
            }
            hasPie ? drawPiePanel(p) : drawXYPanel(p, 550, 1600, 2200, 1200);
        }
        else
        {
            // All panels in one session, each wrapped in endgrf()
            for (auto& p : panels_)
            {
                if (p.series.empty()) continue;
                bool hasPie = false;
                for (auto& s : p.series)
                {
                    if (s.kind == "pie")
                    {
                        hasPie = true;
                        break;
                    }
                }
                auto pg = panelGeom(p.row, p.col);
                hasPie ? drawPiePanel(p) : drawXYPanel(p, pg.x, pg.y, pg.w, pg.h);
                g_->endgrf();
            }
        }

        g_->disfin();
        g_.reset();
    }
};
