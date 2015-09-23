//#include <libxml/xmlexports.h>
#include "generictoplevel.h"
#include "componentdefinition.h"
#include "sequenceannotation.h"

#include <raptor2.h>
#include <unordered_map>

namespace sbol {
	// This is the global SBOL register for classes.  It maps an SBOL RDF type (eg, "http://sbolstandard.org/v2#Sequence" to a constructor
	extern std::unordered_map<std::string, sbol::SBOLObject&(*)()> SBOL_DATA_MODEL_REGISTER;

	template <class SBOLClass>
	void extend_data_model(std::string uri)
	{
		SBOL_DATA_MODEL_REGISTER.insert(make_pair(uri, (SBOLObject&(*)())&create<SBOLClass>));
	};

	
	class Document {
	private:
		raptor_world *rdf_graph;
	public:

		Document() :
			rdf_graph(raptor_new_world())
			{
			};
		std::unordered_map<std::string, sbol::TopLevel*> SBOLObjects;
		std::unordered_map<std::string, sbol::TopLevel*> componentDefinitions;
		std::unordered_map<std::string, sbol::TopLevel*> models;
		std::unordered_map<std::string, sbol::TopLevel*> moduleDefinitions;
		std::unordered_map<std::string, sbol::TopLevel*> sequences;
		std::unordered_map<std::string, sbol::TopLevel*> nameSpaces;

		TopLevel& getTopLevel(std::string);
		raptor_world* getWorld();
		template < class SBOLClass > void add(SBOLClass& sbol_obj);
		template < class SBOLClass > SBOLClass& get(std::string uri);
		void write(std::string filename);
		void read(std::string filename);
		static void parse_objects(void* user_data, raptor_statement* triple);
	};

	// Pitfall:  It's important that the SBOL object represented by sbol_obj is passed by reference not by value!
	template <class SBOLClass > void Document::add(SBOLClass& sbol_obj)
	{
		SBOLObjects[sbol_obj.identity.get()] = (TopLevel*)&sbol_obj;
		sbol_obj.doc = this;
	};

	template <class SBOLClass > SBOLClass& Document::get(std::string uri)
	{
		return (SBOLClass &)*(this->SBOLObjects[uri]);
	};
}

