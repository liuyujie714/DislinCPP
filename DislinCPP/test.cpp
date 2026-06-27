/**
 * test.cpp  —  DislinPlot usage examples
 *
 */

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "DislinPlot.h"

static std::vector<double> linspace(double lo, double hi, int n)
{
    std::vector<double> v(n);
    for (int i = 0; i < n; i++)
        v[i] = lo + i * (hi - lo) / (n - 1);
    return v;
}

// ── 1. Line chart ─────────────────────────────────────────────────────────────
void example_line()
{
    const int           N = 200;
    auto                x = linspace(0, 2 * M_PI, N);
    std::vector<double> ys(N), yc(N);
    for (int i = 0; i < N; i++)
    {
        ys[i] = std::sin(x[i]);
        yc[i] = std::cos(x[i]);
    }

    DislinPlot plt;
    plt.figure("Line Chart", "cons", "da4l");
    plt.plot(x, ys, "red", "sin(x)");
    plt.plot(x, yc, "blue", "cos(x)");
    plt.xlabel("x (radians)");
    plt.ylabel("Amplitude");
    plt.title("Sine and Cosine");
    plt.grid("on");
    plt.axes_bg(0.97, 0.97, 0.97);
    // plt.show();
    plt.savefig("line.svg");
}

// ── 2. Scatter plot ───────────────────────────────────────────────────────────
void example_scatter()
{
    srand(42);
    std::vector<double> x1, y1, x2, y2;
    for (int i = 0; i < 40; i++)
    {
        x1.push_back(1.0 + 0.4 * (rand() / (double)RAND_MAX - 0.5));
        y1.push_back(2.0 + 0.4 * (rand() / (double)RAND_MAX - 0.5));
        x2.push_back(3.0 + 0.4 * (rand() / (double)RAND_MAX - 0.5));
        y2.push_back(4.0 + 0.4 * (rand() / (double)RAND_MAX - 0.5));
    }

    DislinPlot plt;
    plt.figure("Scatter", "cons", "da4l");
    plt.scatter(x1, y1, "red", 0, "Cluster A");
    plt.scatter(x2, y2, "blue", 21, "Cluster B");
    plt.xlabel("X轴");
    plt.ylabel("Y轴");
    plt.title("散点图");
    plt.grid("on");
    plt.symbol_size(20);
    // plt.show();
    plt.savefig("scatter.svg");
    plt.savefig("scatter_chinese.emf");
}

// ── 3. Bar chart ──────────────────────────────────────────────────────────────
void example_bar()
{
    std::vector<double> x = {1, 2, 3, 4, 5, 6, 7};
    std::vector<double> y = {3.2, 5.1, 2.7, 4.8, 3.5, 6.1, 4.3};

    DislinPlot plt;
    plt.figure("Bar Chart", "cons", "da4l");
    plt.bar(x, y, "steelblue", "Monthly Sales");
    plt.xlabel("Month");
    plt.ylabel("Revenue (k)");
    plt.title("Monthly Revenue");
    plt.grid("on");
    plt.axes_bg(0.97, 0.97, 0.97);
    // plt.show();
    plt.savefig("bar.svg");
}

// ── 4. Grouped bar chart ──────────────────────────────────────────────────────
void example_grouped_bar()
{
    std::vector<double>              cats = {1, 2, 3, 4, 5};
    std::vector<std::vector<double>> data = {
        {1.0, 1.5, 2.5, 1.3, 2.0},
        {2.0, 2.7, 3.5, 2.1, 3.2},
        {4.0, 3.5, 4.5, 3.7, 4.0},
    };
    DislinPlot plt;
    plt.figure("Grouped Bars", "cons", "da4l");
    plt.bar_grouped(cats, data, {"red", "green", "blue"}, {"Alpha", "Beta", "Gamma"});
    plt.xlabel("Quarter");
    plt.ylabel("Value");
    plt.ylim(0, 5);
    plt.title("Grouped Bar Chart");
    plt.grid("on");
    // plt.show();
    plt.savefig("groupbar.svg");
}

// ── 5. Pie chart ─────────────────────────────────────────────────────────────
void example_pie()
{
    std::vector<double>      slices = {25, 35, 15, 20, 5};
    std::vector<std::string> labels = {"Python", "C++", "Java", "Rust", "Other"};

    DislinPlot plt;
    plt.figure("Pie", "cons");
    plt.pie(slices, labels);
    plt.title("Language Popularity");
    plt.title_gap(-100);
    // plt.show();
    plt.savefig("pie.svg");
}

// ── 6. Histogram ──────────────────────────────────────────────────────────────
void example_hist()
{
    srand(1234);
    std::vector<double> data;
    for (int i = 0; i < 500; i++)
    {
        double u1 = (rand() + 1.0) / (RAND_MAX + 1.0);
        double u2 = (rand() + 1.0) / (RAND_MAX + 1.0);
        double z  = std::sqrt(-2 * std::log(u1)) * std::cos(2 * M_PI * u2);
        data.push_back(5.0 + 1.5 * z);
    }
    DislinPlot plt;
    plt.figure("Histogram", "cons", "da4l");
    plt.hist(data, 20, "steelblue", "Samples");
    plt.xlabel("Value");
    plt.ylabel("Count");
    plt.title("Histogram - Normal Distribution");
    plt.grid("on");
    plt.axes_bg(0.97, 0.97, 0.97);
    // plt.show();
    plt.savefig("hist.svg");
}

// ── 7. Annotations ───────────────────────────────────────────────────────────
void example_annotated()
{
    auto                x = linspace(-M_PI, M_PI, 150);
    std::vector<double> y(150);
    for (int i = 0; i < 150; i++)
        y[i] = std::sin(x[i]) * std::exp(-0.3 * std::abs(x[i]));

    DislinPlot plt;
    plt.figure("Annotated", "cons", "da4l");
    plt.plot(x, y, "red");
    // Annotations: use coordinates safely inside the data range
    plt.text("Peak", 0.0, 0.9);
    plt.text("Left", -M_PI + 0.2, 0.0);
    plt.text("Right", M_PI - 0.8, 0.0);
    plt.xlabel("x");
    plt.ylabel("f(x)");
    plt.title("Damped Sine with Annotations");
    plt.grid("on");
    // plt.show();
    plt.savefig("annotated.svg");
}

// ── 8. Mixed line + scatter ───────────────────────────────────────────────────
void example_mixed()
{
    const int           N = 30;
    auto                x = linspace(0, 10, N);
    std::vector<double> yfit(N), ymeas(N);
    srand(99);
    for (int i = 0; i < N; i++)
    {
        yfit[i]  = 0.5 * x[i] + 1.0;
        ymeas[i] = yfit[i] + 0.6 * (rand() / (double)RAND_MAX - 0.5);
    }
    DislinPlot plt;
    plt.figure("Mixed", "cons", "da4l");
    plt.scatter(x, ymeas, "blue", 21, "Measured");
    plt.plot(x, yfit, "red", "Fit y=0.5x+1");
    plt.xlabel("Time");
    plt.ylabel("Signal");
    plt.title("Linear Fit vs Measurements");
    plt.grid("on");
    plt.axes_bg(0.97, 0.97, 0.97);
    // plt.show();
    plt.savefig("mixed.svg");
}

// ── 9. Subplots 2×2 ─────────────────────────────────────────────────────────
void example_subplots()
{
    auto                x = linspace(0, 2 * M_PI, 100);
    std::vector<double> s(100), c(100), e(100), sq(100);
    for (int i = 0; i < 100; i++)
    {
        s[i]  = std::sin(x[i]);
        c[i]  = std::cos(x[i]);
        e[i]  = std::exp(-x[i] / (2 * M_PI));
        sq[i] = std::sin(x[i]) * std::sin(x[i]);
    }

    // Subplots: each subplot() call triggers one independent render
    // so we open the Dislin session once and manually position panels.
    // Simpler approach: just call show() four times with different axspos.
    // Here we demonstrate the subplot API:
    DislinPlot plt;
    plt.figure("Subplots", "cons");
    plt.subplot_layout(2, 2);

    plt.subplot(0, 0);
    plt.title("sin(x)");
    plt.plot(x, s, "red");
    plt.xlabel("x1");
    plt.ylabel("y1");
    plt.grid("on");

    plt.subplot(0, 1);
    plt.title("cos(x)");
    plt.plot(x, c, "blue");
    plt.xlabel("x2");
    plt.ylabel("y2");
    plt.grid("on");

    plt.subplot(1, 0);
    plt.title("exp decay");
    plt.plot(x, e, "green");
    plt.xlabel("x3");
    plt.ylabel("y3");
    plt.grid("on");

    plt.subplot(1, 1);
    plt.title("$sin^2(x)$");
    plt.plot(x, sq, "orange");
    plt.xlabel("x4");
    plt.ylabel("y4");
    plt.grid("on");

    // plt.show();
    plt.savefig("subplots.svg");
}

void example_multi_types()
{
    std::vector<std::string> colors = {
        "rgb:1,0,0",   // 红
        "rgb:1,0.5,0", // 橙
        "rgb:1,1,0",   // 黄
        "rgb:0,1,0",   // 绿
        "rgb:0,1,1",   // 青
        "rgb:0,0,1",   // 蓝
        "rgb:0.5,0,1", // 紫
        "rgb:1,0,1"    // 品红
    };
    const size_t ncolors = colors.size();

    DislinPlot plt;
    plt.figure("Multi-Types plot");
    plt.subplot_layout(2, 1);
    int ndata = 8;

    plt.subplot(0, 0);
    for (int i = 0; i < ndata; i++)
    {
        std::vector<double> x = linspace(0, M_PI, 100);
        std::vector<double> y(100);
        std::transform(x.begin(), x.end(), y.begin(), [i](auto s) { return std::cos(s * i + 1); });
        plt.scatter(x, y, colors[i % ncolors], i, "data_" + std::to_string(i));
        plt.symbol_size(10);
    }
    plt.xlim(0, M_PI + 1);
    plt.xlabel("X2");
    plt.ylabel("Y2");
    plt.title("Multi-scatter plot");

    plt.subplot(1, 0);
    std::vector<double>      slices = {25, 35, 15, 20, 5};
    std::vector<std::string> labels = {"Python", "C++", "Java", "Rust", "Other"};
    plt.pie(slices, labels);
    plt.title("Language Popularity");
    plt.title_gap(-100);

    // plt.show();
    plt.savefig("multi_types.svg");
}

int main()
{
    std::cout << "DislinPlot - C++ wrapper for DISLIN\n";

    example_line();
    example_scatter();
    example_bar();
    example_pie();
    example_grouped_bar();
    example_hist();
    example_annotated();
    example_mixed();
    example_subplots();
    example_multi_types();

    return 0;
}
