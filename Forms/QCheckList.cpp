#include "QCheckList.hpp"

QCheckList::QCheckList(QWidget *_parent) : QComboBox(_parent)
{
    model = new QStandardItemModel();
    setModel(model);

    setEditable(true);
    lineEdit()->setReadOnly(true);
    lineEdit()->installEventFilter(this);
    setItemDelegate(new QCheckListStyledItemDelegate(this));

    connect(lineEdit(), &QLineEdit::selectionChanged, lineEdit(), &QLineEdit::deselect);
    connect((QListView*) view(), SIGNAL(pressed(QModelIndex)), this, SLOT(on_itemPressed(QModelIndex)));
    connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)), this, SLOT(on_modelDataChanged()));
}

QCheckList::~QCheckList()
{
    delete model;
}

void QCheckList::SetAllCheckedText(const QString &text)
{
    allCheckedText = text;
    UpdateText();
}

void QCheckList::SetNoneCheckedText(const QString &text)
{
    noneCheckedText = text;
    UpdateText();
}

void QCheckList::SetUnknownlyCheckedText(const QString &text)
{
    unknownlyCheckedText = text;
    UpdateText();
}

QStandardItem *QCheckList::AddCheckItem(const QString &label, const QVariant &data, const Qt::CheckState checkState)
{
    QStandardItem* item = new QStandardItem(label);
    item->setCheckState(checkState);
    item->setData(data);
    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

    model->appendRow(item);

    UpdateText();

    return item;
}

void QCheckList::AddCheckItems(const std::vector<QString> &label, const QVariant &data, const Qt::CheckState checkState)
{
    for (auto i = 0; i < label.size(); i++)
    {
        QStandardItem* item = new QStandardItem(label[i]);
        item->setCheckState(checkState);
        item->setData(data);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

        model->appendRow(item);
    }
    UpdateText();
}

std::vector<bool> QCheckList::GetChecked()
{
    std::vector<bool> result;

    if (GlobalCheckState() == Qt::Unchecked)
    {
        for (auto i = 0; i < model->rowCount(); i++)
        {
            result.push_back(true);
        }
    }
    else
    {
        for (auto i = 0; i < model->rowCount(); i++)
        {
            if (model->item(i)->checkState() == Qt::Checked)
            {
                result.push_back(true);
            }
            else
            {
                result.push_back(false);
            }
        }
    }
    return result;
}

void QCheckList::UncheckAll()
{
    for (auto i = 0; i < model->rowCount(); i++)
    {
        model->item(i)->setData(Qt::Unchecked, Qt::CheckStateRole);
    }
}

/**
 * @brief Computes the global state of the checklist :
 *      - if there is no item: StateUnknown
 *      - if there is at least one item partially checked: StateUnknown
 *      - if all items are checked: Qt::Checked
 *      - if no item is checked: Qt::Unchecked
 *      - else: Qt::PartiallyChecked
 */
int QCheckList::GlobalCheckState()
{
    int nbRows = model->rowCount(), nbChecked = 0, nbUnchecked = 0;

    if (nbRows == 0)
    {
        return StateUnknown;
    }

    for (int i = 0; i < nbRows; i++)
    {
        if (model->item(i)->checkState() == Qt::Checked)
        {
            nbChecked++;
        }
        else if (model->item(i)->checkState() == Qt::Unchecked)
        {
            nbUnchecked++;
        }
        else
        {
            return StateUnknown;
        }
    }

    return nbChecked == nbRows ? Qt::Checked : nbUnchecked == nbRows ? Qt::Unchecked : Qt::PartiallyChecked;
}

bool QCheckList::eventFilter(QObject *_object, QEvent *_event)
{
    if (_object == lineEdit() && _event->type() == QEvent::MouseButtonPress)
    {
        showPopup();
        return true;
    }

    return false;
}

void QCheckList::UpdateText()
{
    QString text;

    switch (GlobalCheckState())
    {
    case Qt::Checked:
        text = allCheckedText;
        break;

    case Qt::Unchecked:
        text = noneCheckedText;
        break;

    case Qt::PartiallyChecked:
        for (int i = 0; i < model->rowCount(); i++)
        {
            if (model->item(i)->checkState() == Qt::Checked)
            {
                if (!text.isEmpty())
                {
                    text+= ", ";
                }

                text+= model->item(i)->text();
            }
        }
        break;

    default:
        text = unknownlyCheckedText;
    }

    lineEdit()->setText(text);
}

void QCheckList::on_modelDataChanged()
{
    UpdateText();
    emit GlobalCheckStateChanged(GlobalCheckState());
}

void QCheckList::on_itemPressed(const QModelIndex &index)
{
    QStandardItem* item = model->itemFromIndex(index);

    if (item->checkState() == Qt::Checked)
    {
        item->setCheckState(Qt::Unchecked);
    }
    else
    {
        item->setCheckState(Qt::Checked);
    }
}

void QCheckList::QCheckListStyledItemDelegate::paint(QPainter *painter_, const QStyleOptionViewItem &option_, const QModelIndex &index_) const
{
    QStyleOptionViewItem & refToNonConstOption = const_cast<QStyleOptionViewItem &>(option_);
    refToNonConstOption.showDecorationSelected = false;
    QStyledItemDelegate::paint(painter_, refToNonConstOption, index_);
}
