#pragma once

#include <QWizardPage>

#include <string>

class Ui_AutoConfigSourcePage;

class AutoConfigSourcePage : public QWizardPage {
	Q_OBJECT

	friend class AutoConfig;

	std::unique_ptr<Ui_AutoConfigSourcePage> ui;

	bool sourceTypeAvailable = false;
	std::string sourceTypeId;

public:
	AutoConfigSourcePage(QWidget *parent = nullptr);
	~AutoConfigSourcePage();

	virtual int nextId() const override;
	virtual bool validatePage() override;

public slots:
	void on_addDisplayCapture_toggled(bool checked);
};
