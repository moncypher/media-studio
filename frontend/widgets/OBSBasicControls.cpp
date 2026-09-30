#include "OBSBasicControls.hpp"
#include "OBSBasic.hpp"
#include "qt-wrappers.hpp"

#include "moc_OBSBasicControls.cpp"

#include <cstring>

static const char *StreamKeySettingName(obs_service_t *service)
{
	const char *protocol = service ? obs_service_get_protocol(service) : nullptr;
	if (protocol && strcmp(protocol, "WHIP") == 0)
		return "bearer_token";

	return "key";
}

OBSBasicControls::OBSBasicControls(OBSBasic *main) : QFrame(nullptr), ui(new Ui::OBSBasicControls), mainWindow(main)
{
	/* Create UI elements */
	ui->setupUi(this);
	ui->streamKeyEdit->setClearButtonEnabled(true);
	streamKeySaveTimer.setSingleShot(true);
	streamKeySaveTimer.setInterval(400);
	connect(&streamKeySaveTimer, &QTimer::timeout, this, &OBSBasicControls::ApplyStreamKey);

	streamButtonMenu.reset(new QMenu());
	startStreamAction = streamButtonMenu->addAction(QTStr("Basic.Main.StartStreaming"));
	stopStreamAction = streamButtonMenu->addAction(QTStr("Basic.Main.StopStreaming"));
	QAction *forceStopStreamAction = streamButtonMenu->addAction(QTStr("Basic.Main.ForceStopStreaming"));

	/* Transfer buttons signals as OBSBasicControls signals */
	connect(
		ui->streamButton, &QPushButton::clicked, this, [this]() { emit this->StreamButtonClicked(); },
		Qt::DirectConnection);
	connect(
		ui->broadcastButton, &QPushButton::clicked, this, [this]() { emit this->BroadcastButtonClicked(); },
		Qt::DirectConnection);
	connect(
		ui->recordButton, &QPushButton::clicked, this, [this]() { emit this->RecordButtonClicked(); },
		Qt::DirectConnection);
	connect(
		ui->pauseRecordButton, &QPushButton::clicked, this, [this]() { emit this->PauseRecordButtonClicked(); },
		Qt::DirectConnection);
	connect(
		ui->replayBufferButton, &QPushButton::clicked, this,
		[this]() { emit this->ReplayBufferButtonClicked(); }, Qt::DirectConnection);
	connect(
		ui->saveReplayButton, &QPushButton::clicked, this,
		[this]() { emit this->SaveReplayBufferButtonClicked(); }, Qt::DirectConnection);
	connect(
		ui->virtualCamButton, &QPushButton::clicked, this, [this]() { emit this->VirtualCamButtonClicked(); },
		Qt::DirectConnection);
	connect(
		ui->virtualCamConfigButton, &QPushButton::clicked, this,
		[this]() { emit this->VirtualCamConfigButtonClicked(); }, Qt::DirectConnection);
	connect(
		ui->modeSwitch, &QPushButton::clicked, this, [this]() { emit this->StudioModeButtonClicked(); },
		Qt::DirectConnection);
	connect(
		ui->settingsButton, &QPushButton::clicked, this, [this]() { emit this->SettingsButtonClicked(); },
		Qt::DirectConnection);
	connect(ui->showStreamKeyButton, &QPushButton::clicked, this, &OBSBasicControls::ToggleStreamKeyVisibility);
	connect(ui->streamKeyEdit, &QLineEdit::editingFinished, this, &OBSBasicControls::ApplyStreamKey);
	connect(ui->streamKeyEdit, &QLineEdit::textChanged, this, [this]() {
		if (!updatingStreamKey)
			streamKeySaveTimer.start();
	});

	/* Transfer menu actions signals as OBSBasicControls signals */
	connect(
		startStreamAction.get(), &QAction::triggered, this,
		[this]() { emit this->StartStreamMenuActionClicked(); }, Qt::DirectConnection);
	connect(
		stopStreamAction.get(), &QAction::triggered, this,
		[this]() { emit this->StopStreamMenuActionClicked(); }, Qt::DirectConnection);
	connect(
		forceStopStreamAction, &QAction::triggered, this,
		[this]() { emit this->ForceStopStreamMenuActionClicked(); }, Qt::DirectConnection);

	/* Set up default visibility — simplified controls only */
	ui->broadcastButton->setVisible(false);
	ui->pauseRecordButton->setVisible(false);
	ui->replayBufferButton->setVisible(false);
	ui->saveReplayButton->setVisible(false);
	ui->virtualCamButton->setVisible(false);
	ui->virtualCamConfigButton->setVisible(false);
	ui->modeSwitch->setVisible(false);

	/* Set up state update connections */
	connect(main, &OBSBasic::StreamingPreparing, this, &OBSBasicControls::StreamingPreparing);
	connect(main, &OBSBasic::StreamingStarting, this, &OBSBasicControls::StreamingStarting);
	connect(main, &OBSBasic::StreamingStarted, this, &OBSBasicControls::StreamingStarted);
	connect(main, &OBSBasic::StreamingStopping, this, &OBSBasicControls::StreamingStopping);
	connect(main, &OBSBasic::StreamingStopped, this, &OBSBasicControls::StreamingStopped);

	connect(main, &OBSBasic::BroadcastStreamReady, this, &OBSBasicControls::BroadcastStreamReady);
	connect(main, &OBSBasic::BroadcastStreamActive, this, &OBSBasicControls::BroadcastStreamActive);
	connect(main, &OBSBasic::BroadcastStreamStarted, this, &OBSBasicControls::BroadcastStreamStarted);

	connect(main, &OBSBasic::RecordingStarted, this, &OBSBasicControls::RecordingStarted);
	connect(main, &OBSBasic::RecordingPaused, this, &OBSBasicControls::RecordingPaused);
	connect(main, &OBSBasic::RecordingUnpaused, this, &OBSBasicControls::RecordingUnpaused);
	connect(main, &OBSBasic::RecordingStopping, this, &OBSBasicControls::RecordingStopping);
	connect(main, &OBSBasic::RecordingStopped, this, &OBSBasicControls::RecordingStopped);

	connect(main, &OBSBasic::ReplayBufStarted, this, &OBSBasicControls::ReplayBufferStarted);
	connect(main, &OBSBasic::ReplayBufStopping, this, &OBSBasicControls::ReplayBufferStopping);
	connect(main, &OBSBasic::ReplayBufStopped, this, &OBSBasicControls::ReplayBufferStopped);

	connect(main, &OBSBasic::VirtualCamStarted, this, &OBSBasicControls::VirtualCamStarted);
	connect(main, &OBSBasic::VirtualCamStopped, this, &OBSBasicControls::VirtualCamStopped);

	connect(main, &OBSBasic::PreviewProgramModeChanged, this, &OBSBasicControls::UpdateStudioModeState);

	/* Set up enablement connection */
	connect(main, &OBSBasic::BroadcastFlowEnabled, this, &OBSBasicControls::EnableBroadcastFlow);
	connect(main, &OBSBasic::ReplayBufEnabled, this, &OBSBasicControls::EnableReplayBufferButtons);
	connect(main, &OBSBasic::VirtualCamEnabled, this, &OBSBasicControls::EnableVirtualCamButtons);
	connect(main, &OBSBasic::StreamServiceChanged, this, &OBSBasicControls::RefreshStreamKey);
}

void OBSBasicControls::RefreshStreamKey()
{
	if (!mainWindow)
		return;

	obs_service_t *service = mainWindow->GetService();
	if (!service)
		return;

	OBSDataAutoRelease settings = obs_service_get_settings(service);
	if (!settings)
		return;

	const QString key = QT_UTF8(obs_data_get_string(settings, StreamKeySettingName(service)));
	if (ui->streamKeyEdit->text() == key)
		return;

	updatingStreamKey = true;
	ui->streamKeyEdit->setText(key);
	updatingStreamKey = false;
}

void OBSBasicControls::ToggleStreamKeyVisibility()
{
	if (ui->streamKeyEdit->echoMode() == QLineEdit::Password) {
		ui->streamKeyEdit->setEchoMode(QLineEdit::Normal);
		ui->showStreamKeyButton->setText(QTStr("Hide"));
		ui->showStreamKeyButton->setToolTip(QTStr("Hide"));
	} else {
		ui->streamKeyEdit->setEchoMode(QLineEdit::Password);
		ui->showStreamKeyButton->setText(QTStr("Show"));
		ui->showStreamKeyButton->setToolTip(QTStr("Show"));
	}
}

void OBSBasicControls::ApplyStreamKey()
{
	streamKeySaveTimer.stop();

	if (!mainWindow || updatingStreamKey)
		return;

	obs_service_t *service = mainWindow->GetService();
	if (!service)
		return;

	OBSDataAutoRelease settings = obs_service_get_settings(service);
	if (!settings)
		return;

	const char *field = StreamKeySettingName(service);
	const QString newKey = ui->streamKeyEdit->text();
	if (newKey == QT_UTF8(obs_data_get_string(settings, field)))
		return;

	obs_data_set_string(settings, field, QT_TO_UTF8(newKey));
	obs_service_update(service, settings);
	mainWindow->SaveService();
}

void OBSBasicControls::SetStreamKeyEnabled(bool enabled)
{
	ui->streamKeyEdit->setEnabled(enabled);
	ui->showStreamKeyButton->setEnabled(enabled);
	ui->streamKeyLabel->setEnabled(enabled);
}

void OBSBasicControls::StreamingPreparing()
{
	ApplyStreamKey();
	SetStreamKeyEnabled(false);

	ui->streamButton->setEnabled(false);
	ui->streamButton->setText(QTStr("Basic.Main.PreparingStream"));
}

void OBSBasicControls::StreamingStarting(bool broadcastAutoStart)
{
	ui->streamButton->setText(QTStr("Basic.Main.Connecting"));

	if (!broadcastAutoStart) {
		// well, we need to disable button while stream is not active
		ui->broadcastButton->setEnabled(false);

		ui->broadcastButton->setText(QTStr("Basic.Main.StartBroadcast"));

		ui->broadcastButton->setProperty("broadcastState", "ready");
		ui->broadcastButton->style()->unpolish(ui->broadcastButton);
		ui->broadcastButton->style()->polish(ui->broadcastButton);
	}
}

void OBSBasicControls::StreamingStarted(bool withDelay)
{
	ui->streamButton->setEnabled(true);
	setClasses(ui->streamButton, "state-active");
	ui->streamButton->setText(QTStr("Basic.Main.StopStreaming"));

	if (withDelay) {
		ui->streamButton->setMenu(streamButtonMenu.get());
		startStreamAction->setVisible(false);
		stopStreamAction->setVisible(true);
	}
}

void OBSBasicControls::StreamingStopping()
{
	ui->streamButton->setText(QTStr("Basic.Main.StoppingStreaming"));
}

void OBSBasicControls::StreamingStopped(bool withDelay)
{
	ui->streamButton->setEnabled(true);
	setClasses(ui->streamButton, "");
	ui->streamButton->setText(QTStr("Basic.Main.StartStreaming"));
	SetStreamKeyEnabled(!withDelay);

	if (withDelay) {
		if (!ui->streamButton->menu()) {
			ui->streamButton->setMenu(streamButtonMenu.get());
		}

		startStreamAction->setVisible(true);
		stopStreamAction->setVisible(false);
	} else {
		ui->streamButton->setMenu(nullptr);
	}
}

void OBSBasicControls::BroadcastStreamReady(bool ready)
{
	setClasses(ui->broadcastButton, ready ? "state-active" : "");
}

void OBSBasicControls::BroadcastStreamActive()
{
	ui->broadcastButton->setEnabled(true);
}

void OBSBasicControls::BroadcastStreamStarted(bool autoStop)
{
	ui->broadcastButton->setText(QTStr(autoStop ? "Basic.Main.AutoStopEnabled" : "Basic.Main.StopBroadcast"));
	if (autoStop) {
		ui->broadcastButton->setEnabled(false);
	}

	ui->broadcastButton->setProperty("broadcastState", "active");
	ui->broadcastButton->style()->unpolish(ui->broadcastButton);
	ui->broadcastButton->style()->polish(ui->broadcastButton);
}

void OBSBasicControls::RecordingStarted(bool pausable)
{
	setClasses(ui->recordButton, "state-active");
	ui->recordButton->setText(QTStr("Basic.Main.StopRecording"));

	if (pausable) {
		ui->pauseRecordButton->setVisible(pausable);
		RecordingUnpaused();
	}
}

void OBSBasicControls::RecordingPaused()
{
	QString text = QTStr("Basic.Main.UnpauseRecording");

	setClasses(ui->pauseRecordButton, "icon-media-pause state-active");
	ui->pauseRecordButton->setAccessibleName(text);
	ui->pauseRecordButton->setToolTip(text);

	ui->saveReplayButton->setEnabled(false);
}

void OBSBasicControls::RecordingUnpaused()
{
	QString text = QTStr("Basic.Main.PauseRecording");

	setClasses(ui->pauseRecordButton, "icon-media-pause");
	ui->pauseRecordButton->setAccessibleName(text);
	ui->pauseRecordButton->setToolTip(text);

	ui->saveReplayButton->setEnabled(true);
}

void OBSBasicControls::RecordingStopping()
{
	ui->recordButton->setText(QTStr("Basic.Main.StoppingRecording"));
}

void OBSBasicControls::RecordingStopped()
{
	setClasses(ui->recordButton, "");
	ui->recordButton->setText(QTStr("Basic.Main.StartRecording"));

	ui->pauseRecordButton->setVisible(false);
}

void OBSBasicControls::ReplayBufferStarted()
{
	setClasses(ui->replayBufferButton, "state-active");
	ui->replayBufferButton->setText(QTStr("Basic.Main.StopReplayBuffer"));

	ui->saveReplayButton->setVisible(true);
}

void OBSBasicControls::ReplayBufferStopping()
{
	ui->replayBufferButton->setText(QTStr("Basic.Main.StoppingReplayBuffer"));
}

void OBSBasicControls::ReplayBufferStopped()
{
	setClasses(ui->replayBufferButton, "");
	ui->replayBufferButton->setText(QTStr("Basic.Main.StartReplayBuffer"));

	ui->saveReplayButton->setVisible(false);
}

void OBSBasicControls::VirtualCamStarted()
{
	setClasses(ui->virtualCamButton, "state-active");
	ui->virtualCamButton->setText(QTStr("Basic.Main.StopVirtualCam"));
}

void OBSBasicControls::VirtualCamStopped()
{
	setClasses(ui->virtualCamButton, "");
	ui->virtualCamButton->setText(QTStr("Basic.Main.StartVirtualCam"));
}

void OBSBasicControls::UpdateStudioModeState(bool enabled)
{
	setClasses(ui->modeSwitch, enabled ? "state-active" : "");
}

void OBSBasicControls::EnableBroadcastFlow(bool enabled)
{
	(void)enabled;
	ui->broadcastButton->setVisible(false);
}

void OBSBasicControls::EnableReplayBufferButtons(bool enabled)
{
	(void)enabled;
	ui->replayBufferButton->setVisible(false);
}

void OBSBasicControls::EnableVirtualCamButtons()
{
	ui->virtualCamButton->setVisible(false);
	ui->virtualCamConfigButton->setVisible(false);
}
