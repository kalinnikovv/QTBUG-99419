#include "mainwindow.h"

#include <QtCore/QAbstractItemModel>
#include <QtCore/QDate>
#include <QtCore/QMimeData>
#include <QtCore/QVariant>
#include <QtCore/QVector>

#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>

namespace Ui {
class MainWindow {
public:
    QFrame* centralwidget;
    QVBoxLayout* verticalLayout;
    QSplitter* splitter;
    QTreeView* leftTreeView;
    QTreeView* rightTreeView;
    QTreeView* bottomTreeView;

    void setupUi(QMainWindow* MainWindow)
    {
        MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1280, 800);
        centralwidget = new QFrame(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        centralwidget->setProperty("orientation", QVariant(4));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        splitter = new QSplitter(centralwidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        leftTreeView = new QTreeView(splitter);
        leftTreeView->setObjectName(QString::fromUtf8("leftTreeView"));
        leftTreeView->setObjectName(QString::fromUtf8("leftTreeView"));
        leftTreeView->header()->setStretchLastSection(true);
        splitter->addWidget(leftTreeView);
        rightTreeView = new QTreeView(splitter);
        rightTreeView->setObjectName(QString::fromUtf8("rightTreeView"));
        rightTreeView->header()->setDefaultSectionSize(24);
        rightTreeView->header()->setSectionResizeMode(QHeaderView::Fixed);
        splitter->addWidget(rightTreeView);

        verticalLayout->addWidget(splitter);

        bottomTreeView = new QTreeView(centralwidget);
        bottomTreeView->setObjectName(QString::fromUtf8("treeView"));

        verticalLayout->addWidget(bottomTreeView);

        MainWindow->setCentralWidget(centralwidget);

        QMetaObject::connectSlotsByName(MainWindow);
    }
};
} // namespace Ui

enum MyTreeColumns {
    NAME,
    EMAIL,
    DEPARTMENT,
    POSITION,
    EMPLOYEENUMBER,
    ADDRESS,
    BIRTHDATE,
    RIGHT_COLUMNS_START,
    RIGHT_COLUMNS_END = RIGHT_COLUMNS_START + 14
};

static constexpr int COLUMN_COUNT = 12;

class MyTreeModel : public QAbstractItemModel {
public:
    MyTreeModel(QObject* parent)
        : QAbstractItemModel(parent)
        , _data {
            {
                "Smith",
                "4023 Church St",
                "smith@exmample.com",
                { 0, 0, 0, 0, 0, 0, 1, 0 },
            },
            {
                "Johnson",
                "1352 Glenview Rd",
                "johnson78@exmample.com",
                { 0, 1, 0, 0, 0, 0, 0, 0 },
            },
        }
    {
    }

    QModelIndex index(int row, int column, const QModelIndex& parent) const
    {
        // Simple 2-level model
        return createIndex(row, column, parent.row());
    }

    QModelIndex parent(const QModelIndex& child) const
    {
        return index(child.internalId(), 0, QModelIndex());
    }

    int rowCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return (parent.internalId() == -1) ? 1 : 0;
        return _data.size();
    }

    int columnCount(const QModelIndex& /*parent*/) const
    {
        return COLUMN_COUNT;
    }

    QVariant data(const QModelIndex& index, int role) const
    {
        // Don't really care about the data, just need a to display *something*

        const auto parent = index.parent();
        if (!parent.isValid()) {
            if (role == Qt::DisplayRole) {
                if (index.column() == NAME)
                    return _data.at(index.row()).name;
                if (index.column() == ADDRESS)
                    return _data.at(index.row()).address;
            }
        } else {
            if (role == Qt::BackgroundRole) {
                const auto& columns = _data[parent.row()].columns;
                if (columns.value(index.column() - RIGHT_COLUMNS_START) > 0)
                    return QBrush(Qt::black, Qt::BDiagPattern);
            }
        }
        return {};
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole)
            return {};

        // Don't really care
        if (section == NAME)
            return tr("Name");
        if (section == DEPARTMENT)
            return tr("Department");
        if (section == POSITION)
            return tr("Position");
        if (section == EMAIL)
            return tr("E-Mail Address");
        if (section == EMPLOYEENUMBER)
            return tr("Employee Number");
        if (section == ADDRESS)
            return tr("Home Address");
        if (section == BIRTHDATE)
            return tr("Date Of Birth");

        return QDate::currentDate().addDays(section - RIGHT_COLUMNS_START).toString("MM.dd");
    }

    Qt::ItemFlags flags(const QModelIndex& index) const
    {
        Qt::ItemFlags flags = Qt::ItemIsEnabled;

        const auto parent = index.parent();
        if (parent.isValid()) {
            const auto& columns = _data[parent.row()].columns;
            // This bug is really only about dragging a cell.
            // Don't care about drop.
            if (columns.value(index.column() - RIGHT_COLUMNS_START) > 0)
                flags |= Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
        }
        return flags;
    }

    Qt::DropActions supportedDropActions() const
    {
        return Qt::CopyAction | Qt::MoveAction;
    }

    QMimeData* mimeData(const QModelIndexList& indexes) const
    {
        // don't care
        return new QMimeData;
    }

private:
    struct Data {
        QString name;
        QString address;
        QString email;

        QVector<int> columns;
    };

    QVector<Data> _data;
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Two tree views aren't necessary, they are here just to demonstrate a use-case
    ui->leftTreeView->setModel(new MyTreeModel(this));
    for (int i = RIGHT_COLUMNS_START; i < RIGHT_COLUMNS_END; ++i)
        ui->leftTreeView->header()->setSectionHidden(i, true);

    ui->rightTreeView->setModel(ui->leftTreeView->model());

    connect(ui->leftTreeView, &QTreeView::expanded, ui->rightTreeView, &QTreeView::expand);
    connect(ui->leftTreeView, &QTreeView::collapsed, ui->rightTreeView, &QTreeView::collapse);
    connect(ui->rightTreeView, &QTreeView::expanded, ui->leftTreeView, &QTreeView::expand);
    connect(ui->rightTreeView, &QTreeView::collapsed, ui->leftTreeView, &QTreeView::collapse);
    ui->leftTreeView->expandAll();

    // This is the crux of it
    ui->rightTreeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->rightTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->rightTreeView->setDragEnabled(true);
    ui->rightTreeView->setDragDropMode(QAbstractItemView::InternalMove);
    for (int i = 0; i < RIGHT_COLUMNS_START; ++i) {
        ui->rightTreeView->header()->setSectionHidden(i, true);
    }

    // The bottom tree view is exactly same as the right one, except the column 0 is not hidden
    ui->bottomTreeView->setModel(ui->rightTreeView->model());
    ui->bottomTreeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->bottomTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->bottomTreeView->setDragEnabled(true);
    ui->bottomTreeView->setDragDropMode(QAbstractItemView::InternalMove);
    for (int i = 1; i < RIGHT_COLUMNS_START; ++i) {
        ui->bottomTreeView->header()->setSectionHidden(i, true);
    }
    ui->bottomTreeView->expandAll();
}

MainWindow::~MainWindow()
{
    delete ui;
}
