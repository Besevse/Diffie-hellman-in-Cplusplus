#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "communication.h"
Communication* worker;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->textEdit->setReadOnly(true);
    ui->textEdit_3->setText("/home/user/input/test.txt");
    ui->textEdit_2->setText("/home/user/output/received.txt");
    ui->textEdit_4->setText("127.0.0.1");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_send_clicked()
{
    worker = new Communication(ui->lineEdit->text().toInt(), ui->lineEdit_2->text().toInt(), ui->textEdit_3->toPlainText(), ui->textEdit_4->toPlainText());
    QThread* thread = new QThread;
    worker->moveToThread(thread);
    connect(thread, SIGNAL(started()), worker, SLOT(send_file()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(text_box(QString)), this, SLOT(change_textBox(QString)));
    thread->start();
}

void MainWindow::on_pushButton_receive_clicked()
{
    worker = new Communication(ui->lineEdit->text().toInt(), ui-> lineEdit_2->text().toInt() ,ui->textEdit_2->toPlainText(), ui->textEdit_4->toPlainText());
    QThread* thread = new QThread;
    worker->moveToThread(thread);
    connect(thread, SIGNAL(started()), worker, SLOT(receive_file()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(text_box(QString)), this, SLOT(change_textBox(QString)));
    thread->start();
}
void MainWindow::change_textBox(QString txt){
    ui->textEdit->setVisible(1);
    ui->textEdit->setText(txt);
}
