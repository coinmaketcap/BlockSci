//
//  address_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "optional_py.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/chain/chain.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/scripts.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

namespace py = pybind11;

using namespace blocksci;

void init_address(py::module &m) {
    py::class_<Address>(m, "Address", "Represents an abstract address object which uniquely identifies a given address")
    .def(py::init<uint32_t, AddressType::Enum>(), "Can be constructed directly by passing it an address index and address type")
    .def("__repr__", &Address::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_readonly("address_num", &Address::addressNum)
    .def_readonly("type", &Address::type)
    .def("outs", py::overload_cast<>(&Address::getOutputs, py::const_), "Returns a list of all outputs sent to this address")
    .def("ins", py::overload_cast<>(&Address::getInputs, py::const_), "Returns a list of all inputs spent from this address")
    .def("txes", py::overload_cast<>(&Address::getTransactions, py::const_), "Returns a list of all transactions involving this address")
    .def("in_txes", py::overload_cast<>(&Address::getInputTransactions, py::const_), "Returns a list of all transaction where this address was an input")
    .def("out_txes", py::overload_cast<>(&Address::getOutputTransactions, py::const_), "Returns a list of all transaction where this address was an output")
    .def_property_readonly("script", py::overload_cast<>(&Address::getScript, py::const_), "Returns the script associated with this address")
    .def_property_readonly("first_tx", py::overload_cast<>(&Address::getFirstTransaction, py::const_), "Get the first transaction that was sent to this address")
    .def_static("address_count", static_cast<size_t(*)()>(addressCount), "Get the total number of address of a given type")
    .def_static("from_string", py::overload_cast<const std::string &>(getAddressFromString), "Construct an address object from an address string")
    ;
    
    py::class_<ScriptPointer>(m, "ScriptPointer", "Represents an abstract script object which uniquely identifies a given script")
    .def(py::init<uint32_t, ScriptType::Enum>(), "Can be constructed directly by passing it an address index and address type")
    .def("__repr__", &ScriptPointer::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_readonly("script_num", &ScriptPointer::scriptNum)
    .def_readonly("type", &ScriptPointer::type)
    .def_property_readonly("script", py::overload_cast<>(&ScriptPointer::getScript, py::const_), "Returns the script associated with this pointer")
    ;
    
    py::class_<Script> baseScript(m, "Script", "Class representing a script which coins are sent to");
    baseScript
    .def("__repr__", py::overload_cast<>(&Script::toString, py::const_))
    .def("__str__", py::overload_cast<>(&Script::toPrettyString, py::const_))
    .def("outs", py::overload_cast<>(&Script::getOutputs, py::const_), "Returns a list of all outputs sent to this script")
    .def("ins", py::overload_cast<>(&Script::getInputs, py::const_), "Returns a list of all inputs spent from this script")
    .def("txes", py::overload_cast<>(&Script::getTransactions, py::const_), "Returns a list of all transactions involving this script")
    .def("in_txes", py::overload_cast<>(&Script::getInputTransactions, py::const_), "Returns a list of all transaction where this script was an input")
    .def("out_txes", py::overload_cast<>(&Script::getOutputTransactions, py::const_), "Returns a list of all transaction where this script was an output")
    ;
    
    py::class_<script::Pubkey>(m, "PubkeyScript", baseScript, "Extra data about pay to pubkey address")
    .def_property_readonly("pubkey", &script::Pubkey::getPubkey, "Public key for this address")
    .def_readonly("pubkeyhash", &script::Pubkey::pubkeyhash, "160 bit address hash")
    .def_property_readonly("address_string", py::overload_cast<>(&script::Pubkey::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::Multisig>(m, "MultisigScript", baseScript, "Extra data about multi-signature address")
    .def_readonly("required", &script::Multisig::required, "The number of signatures required for this address")
    .def_readonly("total", &script::Multisig::total, "The total number of keys that can sign for this address")
    .def_readonly("addresses", &script::Multisig::addresses, "The list of the keys that can sign for this address")
    ;
    
    py::class_<script::ScriptHash>(m, "PayToScriptHashScript", baseScript, "Extra data about pay to script hash address")
    .def_property_readonly("wrapped_address", &script::ScriptHash::getWrappedAddress, "The address inside this P2SH address")
    .def_readonly("raw_address",  &script::ScriptHash::address, "The 160 bit P2SH address hash")
    .def_property_readonly("wrapped_script",py::overload_cast<>(&script::ScriptHash::wrappedScript, py::const_), "The script inside this P2SH address")
    .def_property_readonly("tx_revealed",py::overload_cast<>(&script::ScriptHash::transactionRevealed, py::const_), "The transaction where this wrapped script was first revealed")
    
    .def_property_readonly("address", py::overload_cast<>(&script::ScriptHash::addressString, py::const_), "Bitcoin address string")
    ;
    
    py::class_<script::OpReturn>(m, "NulldataScript", baseScript, "Extra data about op_return address")
    .def_property_readonly("data", [](const script::OpReturn &address) {
        return py::bytes(address.data);
    }, py::return_value_policy::copy, "Data contained inside this script")
    ;
    
    py::class_<script::Nonstandard>(m, "NonStandardScript", baseScript, "Extra data about non-standard address")
    .def_property_readonly("in_script", &script::Nonstandard::inputString, "Nonstandard input script")
    .def_property_readonly("out_script", &script::Nonstandard::outputString, "Nonstandard output script")
    ;
    
    py::enum_<AddressType::Enum>(m, "address_type", py::arithmetic(), "Enumeration of all address types")
    .value("nonstandard", AddressType::Enum::NONSTANDARD)
    .value("pubkey", AddressType::Enum::PUBKEY)
    .value("pubkeyhash", AddressType::Enum::PUBKEYHASH)
    .value("scripthash", AddressType::Enum::SCRIPTHASH)
    .value("multisig", AddressType::Enum::MULTISIG)
    .value("nulldata", AddressType::Enum::NULL_DATA)
    .value("witness_pubkeyhash", AddressType::Enum::WITNESS_PUBKEYHASH)
    .value("witness_scripthash", AddressType::Enum::WITNESS_SCRIPTHASH)
    .export_values()
    .def_property_readonly_static("types", [](py::object) -> std::array<AddressType::Enum, 6> {
        return {{AddressType::Enum::PUBKEY, AddressType::Enum::PUBKEYHASH, AddressType::Enum::SCRIPTHASH,
            AddressType::Enum::MULTISIG, AddressType::Enum::NULL_DATA, AddressType::Enum::NONSTANDARD}};
        })
    .def("pretty_name", [](AddressType::Enum val) {
        switch (val) {
            case AddressType::Enum::PUBKEY:
                return "Pay to pubkey";
            case AddressType::Enum::PUBKEYHASH:
                return "Pay to pubkey hash";
            case AddressType::Enum::SCRIPTHASH:
                return "Pay to script hash";
            case AddressType::Enum::MULTISIG:
                return "Multisig";
            case AddressType::Enum::NONSTANDARD:
                return "Nonstandard";
            case AddressType::Enum::NULL_DATA:
                return "Null data";
            default:
                return "Unknown Address Type";
        }
    })
    ;

}
