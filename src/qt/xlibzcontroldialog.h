// Copyright (c) 2017-2018 The PIVX Developers
// Copyright (c) 2018 The Liberty Developers 
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ZXLIBCONTROLDIALOG_H
#define ZXLIBCONTROLDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "primitives/zerocoin.h"
#include "privacydialog.h"

class CZerocoinMint;
class WalletModel;

namespace Ui {
class XLibzControlDialog;
}

class CXLibzControlWidgetItem : public QTreeWidgetItem
{
public:
    explicit CXLibzControlWidgetItem(QTreeWidget *parent, int type = Type) : QTreeWidgetItem(parent, type) {}
    explicit CXLibzControlWidgetItem(int type = Type) : QTreeWidgetItem(type) {}
    explicit CXLibzControlWidgetItem(QTreeWidgetItem *parent, int type = Type) : QTreeWidgetItem(parent, type) {
    bool operator<(const QTreeWidgetItem &other) const;
};

class XLibzControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit XLibzControlDialog(QWidget *parent);
    ~XLibzControlDialog();

    void setModel(WalletModel* model);

    static std::set<std::string> setSelectedMints;
    static std::set<CMintMeta> setMints;
    static std::vector<CMintMeta> GetSelectedMints();

private:
    Ui::XLibzControlDialog *ui;
    WalletModel* model;
    PrivacyDialog* privacyDialog;

    void updateList();
    void updateLabels();

    enum {
        COLUMN_CHECKBOX,
        COLUMN_DENOMINATION,
        COLUMN_PUBCOIN,
        COLUMN_VERSION,
        COLUMN_CONFIRMATIONS,
        COLUMN_ISSPENDABLE
    };
    
    friend class CXLibzControlWidgetItem;
    
private slots:
    void updateSelection(QTreeWidgetItem* item, int column);
    void ButtonAllClicked();
};

#endif // ZXLIBCONTROLDIALOG_H
