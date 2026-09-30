#include "AutoConfigSourcePage.hpp"
#include "AutoConfig.hpp"
#include "ui_AutoConfigSourcePage.h"

#include <OBSApp.hpp>
#include <widgets/OBSBasic.hpp>

#include <cstring>

#include "moc_AutoConfigSourcePage.cpp"

#define wiz reinterpret_cast<AutoConfig *>(wizard())

namespace {
/* Returns the source id of the platform-appropriate "Display Capture"
 * source, or an empty string if no such source is registered/available. */
std::string GetDisplayCaptureSourceId()
{
#if defined(_WIN32)
	const char *id = "monitor_capture";
#elif defined(__APPLE__)
	const char *id = "display_capture";
#else
	const char *id = "xshm_input";
#endif

	id = obs_get_latest_input_type_id(id);
	if (!id) {
		return std::string();
	}

	OBSProperties props = obs_get_source_properties(id);
	if (!props) {
		return std::string();
	}

	return id;
}
} // namespace

AutoConfigSourcePage::AutoConfigSourcePage(QWidget *parent) : QWizardPage(parent), ui(new Ui_AutoConfigSourcePage)
{
	ui->setupUi(this);

	setTitle(QTStr("Basic.AutoConfig.SourcePage"));
	setSubTitle(QTStr("Basic.AutoConfig.SourcePage.SubTitle"));

	sourceTypeId = GetDisplayCaptureSourceId();
	sourceTypeAvailable = !sourceTypeId.empty();

	if (!sourceTypeAvailable) {
		ui->addDisplayCapture->setChecked(false);
		ui->addDisplayCapture->setEnabled(false);
		ui->monitor->setEnabled(false);
		ui->monitor->clear();
		ui->monitor->addItem(QTStr("Basic.AutoConfig.SourcePage.Unavailable"));
		return;
	}

	OBSProperties props = obs_get_source_properties(sourceTypeId.c_str());
	obs_property_t *monitorProp = obs_properties_first(props);

	if (monitorProp && obs_property_get_type(monitorProp) == OBS_PROPERTY_LIST) {
		size_t count = obs_property_list_item_count(monitorProp);
		for (size_t i = 0; i < count; i++) {
			QString name = QString::fromUtf8(obs_property_list_item_name(monitorProp, i));
			ui->monitor->addItem(name, (int)i);
		}
	}

	/* Default to adding the source only if the current scene doesn't
	 * already have a display/monitor capture source in it. */
	OBSBasic *main = OBSBasic::Get();
	OBSScene scene = main->GetCurrentScene();
	bool alreadyHasSource = false;

	if (scene) {
		obs_scene_enum_items(
			scene,
			[](obs_scene_t *, obs_sceneitem_t *item, void *param) {
				obs_source_t *source = obs_sceneitem_get_source(item);
				const char *id = obs_source_get_unversioned_id(source);
				if (strcmp(id, "monitor_capture") == 0 || strcmp(id, "display_capture") == 0 ||
				    strcmp(id, "xshm_input") == 0) {
					*reinterpret_cast<bool *>(param) = true;
					return false;
				}
				return true;
			},
			&alreadyHasSource);
	}

	ui->addDisplayCapture->setChecked(!alreadyHasSource);
	ui->monitor->setEnabled(!alreadyHasSource);
}

AutoConfigSourcePage::~AutoConfigSourcePage() {}

void AutoConfigSourcePage::on_addDisplayCapture_toggled(bool checked)
{
	ui->monitor->setEnabled(checked && sourceTypeAvailable);
}

int AutoConfigSourcePage::nextId() const
{
	return wiz->type == AutoConfig::Type::VirtualCam ? AutoConfig::TestPage : AutoConfig::VideoPage;
}

bool AutoConfigSourcePage::validatePage()
{
	wiz->addDisplayCaptureSource = sourceTypeAvailable && ui->addDisplayCapture->isChecked();
	wiz->displayCaptureSourceId = sourceTypeId;
	wiz->displayCaptureMonitorIdx = ui->monitor->currentData().toInt();
	wiz->displayCaptureMonitorName = ui->monitor->currentText().toStdString();

	return true;
}
