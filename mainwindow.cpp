#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QDebug>
#include <QMultiMap>
#include <QtMath>
#include "qline.h"

//Function which takes the potential points of the polygon, a specific starting point and an integer k
//and returns the k closest polygon points to the specific starting point.
QVector<QPointF> k_nearest_points(QVector<QPointF> polygon, QPointF point, int k){
    //Initialize multimap to automatically sort points
    QMultiMap<qreal, int> map;
    //Loop through all points in polygon
    for(int i = 0;i < polygon.length(); i++){
        //Calculate x and y distances from point to polygon point
        qreal x = polygon[i].x() - point.x();
        qreal y = polygon[i].y() - point.y();
        //Ensure point itself is not one of it's own closest points
        if(x == 0 && y == 0){
            continue;
        }
        //Insert square of distance into multimap to be sorted
        map.insert((x*x + y*y), i);
    }
    //Initialize list of k nearest points to return
    QVector<QPointF> result;
    //Loop through sorted points while k > 0
    for(auto i = map.begin(); i != map.end() && k>0; i++, k--){
        //Add next closest point to result
        result.append(polygon[i.value()]);
    }
    return result;
}

//Function which takes a number of points
//and returns the point with the lowest y value
//If there are multiple tied lowest points, this function will return one of them.
//In a QT window this will be the top point
QPointF min_y(QVector<QPointF> points){
    //Initialize multimap to automatically sort points
    QMultiMap<qreal, int> map;
    //For point in points
    for(int i = 0; i < points.length(); i++){
        //Insertinto multimap based on y coordinate
        map.insert(points[i].y(), i);
    }
    //Return lowest y value coordinate
    return points[map.begin().value()];

}


QPointF start = QPointF();
QPointF end = QPointF();
//Function which takes a number of points, a specific starting point and a previous point
//and returns the rightmost point. This is decided by the smallest positive distance from the inverse of the starting angle.
QPointF right_most_point(QVector<QPointF> points, QPointF point, QPointF previous_point){
    //Initialize multimap to automatically sort points
    QMultiMap<qreal, int> map;
    //Initialize components of and angle of previous point to the current point
    qreal previous_x = point.x() - previous_point.x();
    qreal previous_y = point.y() - previous_point.y();
    qreal previous_angle = qAtan2(previous_x, previous_y);
    //For point in points
    for(int i = 0; i < points.length(); i++){
        //Initialize components of and angle of current point to component of points
        qreal x = points[i].x() - point.x();
        qreal y = points[i].y() - point.y();
        qreal angle = qAtan2(x, y) - previous_angle;
        //If angle is negative take it away from 2PI to get larger positive angle
        while(angle < 0){
            angle = 2*M_PI + angle;
        }
        //Insert angle into sorted multimap
        map.insert(angle, i);
    }
    //Return angle with smallest positive angle value
    return points[map.begin().value()];
}

//A function which takes a vector of lines making up a polygon and a single line
//and returns whether the single line crosses the line of the polygon
bool line_does_not_intersect(QVector<QLineF> lines, QLineF line){
    for(int it = 0; it < lines.size(); it++){
        if(lines[it].intersect(line, nullptr) == QLineF::BoundedIntersection
                //Account for previous lines that should not be counted as intersecting
                && lines[it].p1() != line.p1() && lines[it].p1() != line.p1()
                && lines[it].p2() != line.p2() && lines[it].p2() != line.p2()){
            //qDebug() << "Intersection Detected" << lines[it] << line;
            //I am lazy and just put two constantly updating global variables instead of changing up my code
            start = lines[it].p2();
            end = line.p2();
            return false;
        }
    }
    return true;

}

//A function that takes a vector of QPointFs
//and returns a concave polygon wrapping the provided points
QPolygonF concave_hull(QVector<QPointF> &points, int k){

    //Initialize starting point
    QPointF current_point = min_y(points);
    //Initialize previous point for starting angle of 270o
    QPointF  previous_point = QPointF(current_point.x()  + 100, current_point.y());
    //Initialize resulting polygon
    QPolygonF result;
    //Initialize intermediate point to hold potential next point while it is determined whether the next point crosses the polygon
    QPointF intermediate_point;
    //Add initial start to result
    result.append(current_point);
    //Initialize vector of lines for the purpose of detecting intersections
    QVector<QLineF> lines;
    //While polygon is not closed and larger then a single point
   while(!result.isClosed() || result.size() == 1){
        //Find nearest points
        QVector<QPointF> nearest_points  = k_nearest_points(points, current_point, k);
        //If no nearest points are found
        if(nearest_points.isEmpty()){
            //Polygon is only a single point
            qDebug() << "Only one point in polygon";
            return result;
        }
        //Loop through possible next points in nearest_points
        do{
            intermediate_point = right_most_point(nearest_points, current_point, previous_point);
            nearest_points.removeOne(intermediate_point);
            //Continue looping if point intersects
        }while(!line_does_not_intersect(lines, QLineF(current_point, intermediate_point)) && nearest_points.size() > 1);

        //Update value of previous and current points
        previous_point = current_point;
        current_point = intermediate_point;
        //Add newly formed line to lines vector for intersection checking
        lines.append(QLineF(current_point, previous_point));
        //Add new point onto result polygon
        result.append(current_point);

        //Cleanup stage
        //If failed to find non intersecting way through:
        if(nearest_points.size() == 1){
            qDebug() << "Unavoidable intercept: Initiating simplification";
            //Loop that removes all points from polygon between two tangled points
            for(int i = result.indexOf(end) - 1; i>result.indexOf(start); i--){
                result.remove(i);
            }

        }

        //Remove newly added point from pool of available points
        points.removeOne(current_point);

    }

    return result;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event){

    QPainter painter(this);
    QPen pen(Qt::black, 5, Qt::SolidLine);
    QBrush brush(Qt::red, Qt::VerPattern);
    painter.setPen(pen);
    painter.setBrush(brush);

    QVector<QPointF> points;

    points << QPointF(600,500) << QPointF(300,350) << QPointF(300,250) << QPointF(400,150) << QPointF(600,100)
           << QPointF(400, 50) << QPointF(200, 100) << QPointF(100,200) << QPointF(200, 300) << QPointF(200, 400)
           << QPointF(300, 450);

    points << QPointF(300,200) << QPointF(400, 480) << QPointF(500, 490) << QPointF(500, 400) << QPointF(550, 450)
           << QPointF(500, 350) << QPointF(450, 300) << QPointF(500, 200) << QPointF(500, 150) << QPointF(500, 50)
           << QPointF(300, 75) << QPointF(100, 300) << QPointF(200, 200) << QPointF(500, 250) << QPointF(600, 400);

    points << QPointF(700, 450) << QPointF(450, 450) << QPointF(550, 175) << QPointF(600, 55);

    QVector<QPointF> points_copy = points;

    QPolygonF polygon = concave_hull(points,4);
    painter.drawPolygon(polygon);

    pen.setColor(Qt::blue);
    pen.setWidth(10);
    painter.setPen(pen);
    painter.drawPoints(points_copy);

}
