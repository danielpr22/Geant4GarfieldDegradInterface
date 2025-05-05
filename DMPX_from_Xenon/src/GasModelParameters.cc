#include "../include/GasModelParameters.hh"
#include "../include/DegradModel.hh"
#include "../include/GasModelParametersMessenger.hh"
#include "../include/DetectorConstruction.hh"

GasModelParameters::GasModelParameters() {
	fMessenger = new GasModelParametersMessenger(this);
}
