/**
 * @file    dbtl.cpp
 * @brief   Methods for Design-Build-Test classes
 * @author  Bryan Bartley
 * @email   bartleyba@sbolstandard.org
 *
 * <!--------------------------------------------------------------------------
 * This file is part of libSBOL.  Please visit http://sbolstandard.org for more
 * information about SBOL, and the latest version of libSBOL.
 *
 *  Copyright 2016 University of Washington, WA, USA
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------>*/

#include "dbtl.h"
#include "document.h"

using namespace std;
using namespace sbol;

namespace sbol
{
    Design::~Design()
    {
        // Remove these properties to prevent double deletion of child objects by the base destructor
        owned_objects.erase(SBOL_COMPONENT_DEFINITION);
        owned_objects.erase(SBOL_MODULE_DEFINITION);
    }
    
    Build::~Build()
    {
        // Remove these properties to prevent double deletion of child objects by the base destructor
        owned_objects.erase(SBOL_COMPONENT_DEFINITION);
        owned_objects.erase(SBOL_MODULE_DEFINITION);
    }
    
    Analysis::~Analysis()
    {
        // Remove these properties to prevent double deletion of child objects by the base destructor
        owned_objects.erase(SBOL_SEQUENCE);
        owned_objects.erase(SBOL_MODEL);
    }

/* Specialized templates for Design, Build, Test, Analysis workflows */
    
    template<>
    Design& TopLevel::generate<Design>(std::string uri)
    {
        if (Config::getOption("sbol_compliant_uris") != "True")
            throw SBOLError(SBOL_ERROR_COMPLIANCE, "This method signature must only be used in SBOL-compliant mode. Use one of the other generate method signatures or use Config::setOption to enable SBOL-compliant URIs");
        
        if (Config::getOption("sbol_typed_uris") != "True")
            throw SBOLError(SBOL_ERROR_COMPLIANCE, "This method signature must only be used with SBOL-typed URIs. Use one of the other generate method signatures or use Config::setOption to enable SBOL-typed URIs");
        
        if (doc == NULL)
            throw SBOLError(SBOL_ERROR_MISSING_DOCUMENT, "Generate method requires that the progenitor object belong to a Document.");
        
        if (this->type != SYSBIO_ANALYSIS && this->type != SYSBIO_DESIGN)
            throw SBOLError(SBOL_ERROR_INVALID_ARGUMENT, "Object " + this->identity.get() + " cannot generate a Design. A Design can only be generated from an Analysis or another Design.");
        
        Design& design = *new Design(uri);
        design.wasDerivedFrom.set(this->identity.get());
        if (this->type == SYSBIO_ANALYSIS)
            design.characterization.set(this->identity.get());
        
        // Form URI for auto-constructing an Activity
        std::string id;
        if (Config::getOption("sbol_compliant_uris") == "True")
            id = design.displayId.get();
        else
            id = design.identity.get();
        
        // Auto-construct Activity
        Activity& a = doc->activities.create(id + "_generation");
        design.wasGeneratedBy.set(a);
        
        // Form URI for auto-constructing a Usage referring to the generating object
        if (Config::getOption("sbol_compliant_uris") == "True")
            id = this->displayId.get();
        else
            id = this->identity.get();
        Usage& u = a.usages.create(id + "_usage");
        u.entity.set(this->identity.get());
        
        // The generated Design may use an Analysis to learn, or else it uses a Design to design, depending on which type of object generated it.
        if (this->type == SYSBIO_ANALYSIS)
            u.roles.add(SBOL_LEARN);
        else
            u.roles.add(SBOL_DESIGN);
        
        doc->add<Design>(design);
        return design;
    }
 
    template<>
    Design& TopLevel::generate<Design>(std::string uri, Agent& agent, Plan& plan, std::vector < Identified* > usages)
    {
        for (auto & obj : usages)
            if (obj->type != SYSBIO_ANALYSIS && obj->type != SYSBIO_DESIGN)
                throw SBOLError(SBOL_ERROR_INVALID_ARGUMENT, "Invalid usages argument. A Design may only use other Designs or Analysis objects for generation");

        Design& design = TopLevel::generate<Design>(uri);
        Activity& a = doc->get<Activity>(design.wasGeneratedBy.get());
    
        // Form URI for auto-constructing an Association
        std::string id;
        if (Config::getOption("sbol_compliant_uris") == "True")
            id = design.displayId.get();
        else
            id = design.identity.get();
        
        // Auto-construct Association
        Association& asc = a.associations.create(id + "_generation_association");
        asc.roles.set(SBOL_URI "#design");
        asc.agent.set(plan);
        asc.plan.set(plan);
    
        for (auto & usage : usages)
        {
            if (Config::getOption("sbol_compliant_uris") == "True")
                id = usage->displayId.get();
            else
                id = usage->identity.get();
            Usage& u = a.usages.create(id + "_usage");
            u.entity.set(usage->identity.get());
            if (usage->type == SYSBIO_ANALYSIS)
            {
                u.roles.set(SBOL_LEARN);
                design.characterization.add(u);
            }
            else
                u.roles.set(SBOL_DESIGN);
        }
        return design;
    };
    
    
    template<>
    Build& TopLevel::generate<Build>(std::string uri)
    {
        if (Config::getOption("sbol_compliant_uris") != "True")
            throw SBOLError(SBOL_ERROR_COMPLIANCE, "This method signature must only be used in SBOL-compliant mode. Use one of the other generate method signatures or use Config::setOption to enable SBOL-compliant URIs");
    
        if (Config::getOption("sbol_typed_uris") != "True")
            throw SBOLError(SBOL_ERROR_COMPLIANCE, "This method signature must only be used with SBOL-typed URIs. Use one of the other generate method signatures or use Config::setOption to enable SBOL-typed URIs");
    
        if (doc == NULL)
            throw SBOLError(SBOL_ERROR_MISSING_DOCUMENT, "Generate method requires that the progenitor object belong to a Document.");
    
        if (this->type != SBOL_IMPLEMENTATION && this->type != "http://sys-bio.org#Design")
            throw SBOLError(SBOL_ERROR_INVALID_ARGUMENT, "A Build can only be generated from a Design or Build.");
    
        Build& build = *new Build(uri);
        build.wasDerivedFrom.set(this->identity.get());
        build.design.set(this->identity.get());

        // Form URI for auto-constructing an Activity
        std::string id;
        if (Config::getOption("sbol_compliant_uris") == "True")
            id = build.displayId.get();
        else
            id = build.identity.get();
        
        // Auto-construct Activity
        Activity& a = doc->activities.create(id + "_generation");
        build.wasGeneratedBy.set(a);

        // Form URI for auto-constructing a Usage referring to the generating object
        if (Config::getOption("sbol_compliant_uris") == "True")
            id = this->displayId.get();
        else
            id = this->identity.get();
        Usage& u = a.usages.create(id + "_usage");
        u.entity.set(this->identity.get());
        
        // The generated Build may use a Design to design, or else it uses a Build to build, depending on which type of object generated it.
        if (this->type == "http://sys-bio.org#Design")
            u.roles.set(SBOL_URI "#design");
        else
            u.roles.set(SBOL_URI "#build");

        doc->add<Build>(build);
        return build;
    }
    
    template<>
    Build& TopLevel::generate<Build>(std::string uri, Agent& agent, Plan& plan, std::vector < Identified* > usages)
    {
        for (auto & obj : usages)
            if (obj->type != SBOL_IMPLEMENTATION)
                throw SBOLError(SBOL_ERROR_INVALID_ARGUMENT, "Invalid usages argument. A Build may only use other Builds for generation");

        Build& build = TopLevel::generate<Build>(uri);
        Activity& a = doc->get<Activity>(build.wasGeneratedBy.get());
    
        // Form URI for auto-constructing an Association
        std::string id;
        if (Config::getOption("sbol_compliant_uris") == "True")
            id = build.displayId.get();
        else
            id = build.identity.get();
        
        // Auto-construct an Association
        Association& asc = a.associations.create(id + "_generation_association");
        asc.roles.set(SBOL_URI "#build");
        asc.agent.set(agent);
        asc.plan.set(plan);
    
        for (auto & usage : usages)
        {
            if (Config::getOption("sbol_compliant_uris") == "True")
                id = usage->displayId.get();
            else
                id = usage->identity.get();
            Usage& u = a.usages.create(id + "_usage");
            u.entity.set(usage->identity.get());
            u.roles.set(SBOL_URI "#build");
        }
        return build;
    };
    
    template<>
    Test& TopLevel::generate<Test>(std::string uri)
    {
        if (Config::getOption("sbol_compliant_uris") != "True")
            throw SBOLError(SBOL_ERROR_COMPLIANCE, "This method signature must only be used in SBOL-compliant mode. Use one of the other generate method signatures or use Config::setOption to enable SBOL-compliant URIs");
        
        if (Config::getOption("sbol_typed_uris") != "True")
            throw SBOLError(SBOL_ERROR_COMPLIANCE, "This method signature must only be used with SBOL-typed URIs. Use one of the other generate method signatures or use Config::setOption to enable SBOL-typed URIs");
        
        if (doc == NULL)
            throw SBOLError(SBOL_ERROR_MISSING_DOCUMENT, "Generate method requires that the progenitor object belong to a Document.");
        
        if (this->type != SBOL_IMPLEMENTATION && this->type != SBOL_COLLECTION)
            throw SBOLError(SBOL_ERROR_INVALID_ARGUMENT, "A Test can only be generated from a Build or another Test.");

        Test& test = *new Test(uri);
        test.wasDerivedFrom.set(this->identity.get());

        // Set Test.samples property
        if (properties.find(SYSBIO_URI "#type") != properties.end())
        {
            if (properties[SYSBIO_URI "#type"].front() == "<" SYSBIO_URI "#SampleRoster>")
            {
                SampleRoster& roster = *(SampleRoster*)this;
                for (auto & sample_id : roster.samples)
                    test.samples.add(sample_id);
            }
            else if (properties[SYSBIO_URI "#type"].front() == "<" SYSBIO_BUILD ">")
            {
                // The progenitor object is either a Test or Build
                test.samples.set(this->identity.get());
            }
        }
        
        // Form URI for auto-constructing an Activity
        std::string id;
        if (Config::getOption("sbol_compliant_uris") == "True")
            id = test.displayId.get();
        else
            id = test.identity.get();
        
        // Auto-construct Activity
        Activity& a = doc->activities.create(id + "_generation");
        test.wasGeneratedBy.set(a);
        
        // Form URI for auto-constructing a Usage referring to the generating object
        if (Config::getOption("sbol_compliant_uris") == "True")
            id = this->displayId.get();
        else
            id = this->identity.get();
        
        Usage& u = a.usages.create(id + "_usage");
        u.entity.set(this->identity.get());
        
        // The generated Build may use a Design to design, or else it uses a Build to build, depending on which type of object generated it.
        if (this->type == SBOL_IMPLEMENTATION)
            u.roles.set(SBOL_URI "#build");
        else
            u.roles.set(SBOL_URI "#test");
 
        doc->add<Test>(test);
        return test;
    }

    template<>
    Test& TopLevel::generate<Test>(std::string uri, Agent& agent, Plan& plan, std::vector < Identified* > usages)
    {
        for (auto & obj : usages)
            if (obj->type != SBOL_IMPLEMENTATION && obj->type != SBOL_COLLECTION)
                throw SBOLError(SBOL_ERROR_INVALID_ARGUMENT, "A Test may only use Builds or other Tests for generation");

        Test& test = TopLevel::generate<Test>(uri);
        Activity& a = doc->get<Activity>(test.wasGeneratedBy.get());
    
        std::string id;
        if (Config::getOption("sbol_compliant_uris") == "True")
            id = test.displayId.get();
        else
            id = test.identity.get();
        Association& asc = a.associations.create(id + "_generation_association");
        asc.plan.set(plan);
        asc.roles.set(SBOL_URI "#test");
    
        // Check if the progenitor object is a SampleRoster (collection of Builds)
        if (properties.find(SYSBIO_URI "#type") != properties.end())
            if (properties[SYSBIO_URI "#type"].front() == "<" SYSBIO_URI "#SampleRoster>")
            {
                std::cout << "Generating from SampleRoster" << std::endl;
                SampleRoster& roster = *(SampleRoster*)this;
                for (auto & sample_id : roster.samples)
                {
                    Identified& sample = this->doc->get<Identified>(sample_id);
                    usages.push_back(&sample);
                }
            }

        for (auto & usage : usages)
        {
            if (Config::getOption("sbol_compliant_uris") == "True")
                id = usage->displayId.get();
            else
                id = usage->identity.get();
            Usage& u = a.usages.create(id + "_usage");
            u.entity.set(usage->identity.get());
            if (usage->type == SBOL_IMPLEMENTATION)
            {
                u.roles.set(SBOL_BUILD);
                test.samples.add(u);
            }
            else
                u.roles.set(SBOL_TEST);
        }
        return test;
    };
    
    template<>
    Analysis& TopLevel::generate<Analysis>(std::string uri)
    {
        if (Config::getOption("sbol_compliant_uris") != "True")
            throw SBOLError(SBOL_ERROR_COMPLIANCE, "This method signature must only be used in SBOL-compliant mode. Use one of the other generate method signatures or use Config::setOption to enable SBOL-compliant URIs");
        
        if (Config::getOption("sbol_typed_uris") != "True")
            throw SBOLError(SBOL_ERROR_COMPLIANCE, "This method signature must only be used with SBOL-typed URIs. Use one of the other generate method signatures or use Config::setOption to enable SBOL-typed URIs");
        
        if (doc == NULL)
            throw SBOLError(SBOL_ERROR_MISSING_DOCUMENT, "Generate method requires that the progenitor object belong to a Document.");
        
        if (this->type != SBOL_COLLECTION && this->type != SYSBIO_ANALYSIS)
            throw SBOLError(SBOL_ERROR_INVALID_ARGUMENT, "An Analysis can only be generated from a Test or another Analysis.");
        
        Analysis& analysis = *new Analysis(uri);
        analysis.wasDerivedFrom.set(this->identity.get());
        analysis.rawData.set(this->identity.get());
        
        std::string id;
        if (Config::getOption("sbol_compliant_uris") == "True")
            id = analysis.displayId.get();
        else
            id = analysis.identity.get();
        Activity& a = doc->activities.create(analysis.displayId.get() + "_generation");
        analysis.wasGeneratedBy.set(a);
        
        if (Config::getOption("sbol_compliant_uris") == "True")
            id = this->displayId.get();
        else
            id = this->identity.get();
        Usage& u = a.usages.create(id + "_usage");
        u.entity.set(this->identity.get());
        if (this->type == SBOL_COLLECTION)
            u.roles.set(SBOL_URI "#test");
        else
            u.roles.set(SBOL_URI "#learn");
        
        doc->add<Analysis>(analysis);
        return analysis;
    }
    
    template<>
    Analysis& TopLevel::generate<Analysis>(std::string uri, Agent& agent, Plan& plan, std::vector < Identified* > usages)
    {
        for (auto & obj : usages)
            if (obj->type != SYSBIO_ANALYSIS)
                throw SBOLError(SBOL_ERROR_INVALID_ARGUMENT, "An Analysis may only use other Analysis objects for generation");
        
        Analysis& analysis = TopLevel::generate<Analysis>(uri);
        Activity& a = doc->get<Activity>(analysis.wasGeneratedBy.get());
        
        std::string id;
        if (Config::getOption("sbol_compliant_uris") == "True")
            id = analysis.displayId.get();
        else
            id = analysis.identity.get();
        Association& asc = a.associations.create(id + "_generation_association");
        asc.roles.set(SBOL_URI "#learn");
        asc.agent.set(agent);
        asc.plan.set(plan);
        
        for (auto & usage : usages)
        {
            if (Config::getOption("sbol_compliant_uris") == "True")
                id = usage->displayId.get();
            else
                id = usage->identity.get();
            Usage& u = a.usages.create(id + "_usage");
            u.entity.set(usage->identity.get());
            u.roles.set(SBOL_URI "#learn");
        }
        return analysis;
    };
    
    
    template<>
    Design& OwnedObject<Design>::get(std::string uri)
    {
        Design& design = (Design&)get<TopLevel>(uri);
        Document* doc = this->sbol_owner->doc;
        if (doc)
        {
            if (!design.structure.size() && design._structure.size())
                if (doc->find(design._structure.get()))
                    design.structure.set(doc->get<ComponentDefinition>(design._structure.get()));
            if (!design.function.size() && design._function.size())
                if (doc->find(design._function.get()))
                    design.function.set(doc->get<ModuleDefinition>(design._function.get()));
        }
        return design;
    }

    template<>
    Build& OwnedObject<Build>::get(std::string uri)
    {
        Build& build = (Build&)get<TopLevel>(uri);
        Document* doc = this->sbol_owner->doc;
        if (doc)
        {
            if (!build.structure.size() && build._structure.size())
                if (doc->find(build._structure.get()))
                    build.structure.set(doc->get<ComponentDefinition>(build._structure.get()));
            if (!build.behavior.size() && build._behavior.size())
                if (doc->find(build._behavior.get()))
                    build.behavior.set(doc->get<ModuleDefinition>(build._behavior.get()));
        }
        return build;
    }
    
    template<>
    Design& Document::get<Design>(std::string uri)
    {
        Design& design = (Design&)get<TopLevel>(uri);
        if (!design.structure.size() && design._structure.size())
            if (this->find(design._structure.get()))
                design.structure.set(doc->get<ComponentDefinition>(design._structure.get()));
        if (!design.function.size() && design._function.size())
            if (this->find(design._function.get()))
                design.function.set(doc->get<ModuleDefinition>(design._function.get()));
        return design;
    }
    
    template<>
    Build& Document::get<Build>(std::string uri)
    {
        Build& build = (Build&)get<TopLevel>(uri);
        if (!build.structure.size() && build._structure.size())
            if (this->find(build._structure.get()))
                build.structure.set(doc->get<ComponentDefinition>(build._structure.get()));
        if (!build.behavior.size() && build._behavior.size())
            if (this->find(build._behavior.get()))
                build.behavior.set(doc->get<ModuleDefinition>(build._behavior.get()));
        return build;
    }
    
    // The Build class requires a specialized adder at the Document level. Since its rdf:type is identical to Implementation, it will get added to the Implementation store by default. It must be moved to the Build store.
    template<>
    void Document::add<Build>(Build& sbol_obj)
    {
        // Call recursive adder
        add<Implementation>((Implementation&)sbol_obj);

        // Move from Implementation store to Build store
        this->owned_objects[SBOL_IMPLEMENTATION].pop_back();
        this->owned_objects[SYSBIO_BUILD].push_back(&sbol_obj);

    }

    template<>
    void Document::add<Test>(Test& sbol_obj)
    {
        // Call recursive adder
        add<Collection>((Collection&)sbol_obj);
        
        // Move from Collection store to Test store
        this->owned_objects[SBOL_COLLECTION].pop_back();
        this->owned_objects[SYSBIO_TEST].push_back(&sbol_obj);
    }
    
//    template<>
//    Build& OwnedObject<Build>::get(std::string uri)
//    {
//        
//    }
//
//    template<>
//    Build& OwnedObject<Build>::set(Build& sbol_obj)
//    {
//        
//    }
//
//    template<>
//    Build& OwnedObject<Build>::add(Build& sbol_obj)
//    {
//        
//    }
//    
    template<>
    Build& OwnedObject<Build>::create(std::string uri)
    {
        Build* child_obj = new Build();

        SBOLObject* parent_obj = this->sbol_owner;
        Document* parent_doc = this->sbol_owner->doc;

        if (Config::getOption("sbol_compliant_uris").compare("True") == 0)
        {
            string persistent_id = getHomespace();
            if (Config::getOption("sbol_typed_uris").compare("True") == 0)
                persistent_id += "/" + parseClassName(SYSBIO_BUILD);
            string version = VERSION_STRING;
            
            string child_persistent_id =  persistent_id + "/" + uri;
            string child_id = child_persistent_id + "/" + version;
            
            // Check for uniqueness of URI in the Document
            if (parent_doc && parent_doc->find(child_id))
                throw SBOLError(DUPLICATE_URI_ERROR, "An object with URI " + child_id + " is already in the Document");
                                                      
            // Initialize SBOLCompliant properties
            child_obj->identity.set(child_id);
            child_obj->persistentIdentity.set(child_persistent_id);
            child_obj->displayId.set(uri);
            child_obj->version.set(version);

            // Add the new object to this OwnedObject property
            // this->add(*child_obj);   Can't use this because the add method is prohibited in SBOLCompliant mode!!!
            child_obj->parent = parent_obj;  // Set back-pointer to parent object
            std::vector< sbol::SBOLObject* >& object_store = this->sbol_owner->owned_objects[this->type];
            object_store.push_back((SBOLObject*)child_obj);

            // The following effectively adds the child object to the Document by setting its back-pointer.  However, the Document itself only maintains a register of TopLevel objects, otherwise the returned object will not be registered
            if (parent_doc)
            {
                child_obj->doc = parent_doc;
                parent_doc->SBOLObjects[child_id] = (SBOLObject*)child_obj;
            }
            this->validate(child_obj);
            return *child_obj;
        }
        else
        {
            if (parent_doc && parent_doc->find(uri))
                throw SBOLError(DUPLICATE_URI_ERROR, "An object with URI " + uri + " is already in the Document");
            
            // Add the new object to this OwnedObject property
            Identified* parent_obj = (Identified*)this->sbol_owner;
            child_obj->parent = parent_obj;  // Set back-pointer to parent object
            child_obj->identity.set(uri);
            child_obj->persistentIdentity.set(uri);
            
            this->add(*child_obj);
            if (parent_doc)
                child_obj->doc = parent_doc;
            this->validate(child_obj);
            return *child_obj;
        }
    }

    template<>
    Test& OwnedObject<Test>::create(std::string uri)
    {
        Test* child_obj = new Test();

        SBOLObject* parent_obj = this->sbol_owner;
        Document* parent_doc = this->sbol_owner->doc;

        if (Config::getOption("sbol_compliant_uris").compare("True") == 0)
        {
            string persistent_id = getHomespace();
            if (Config::getOption("sbol_typed_uris").compare("True") == 0)
                persistent_id += "/" + parseClassName(SYSBIO_TEST);
            string version = VERSION_STRING;
            
            string child_persistent_id =  persistent_id + "/" + uri;
            string child_id = child_persistent_id + "/" + version;
            
            // Check for uniqueness of URI in the Document
            if (parent_doc && parent_doc->find(child_id))
                throw SBOLError(DUPLICATE_URI_ERROR, "An object with URI " + child_id + " is already in the Document");
                                                      
            // Initialize SBOLCompliant properties
            child_obj->identity.set(child_id);
            child_obj->persistentIdentity.set(child_persistent_id);
            child_obj->displayId.set(uri);
            child_obj->version.set(version);

            // Add the new object to this OwnedObject property
            // this->add(*child_obj);   Can't use this because the add method is prohibited in SBOLCompliant mode!!!
            child_obj->parent = parent_obj;  // Set back-pointer to parent object
            std::vector< sbol::SBOLObject* >& object_store = this->sbol_owner->owned_objects[this->type];
            object_store.push_back((SBOLObject*)child_obj);

            // The following effectively adds the child object to the Document by setting its back-pointer.  However, the Document itself only maintains a register of TopLevel objects, otherwise the returned object will not be registered
            if (parent_doc)
            {
                child_obj->doc = parent_doc;
                parent_doc->SBOLObjects[child_id] = (SBOLObject*)child_obj;
            }
            this->validate(child_obj);
            return *child_obj;
        }
        else
        {
            if (parent_doc && parent_doc->find(uri))
                throw SBOLError(DUPLICATE_URI_ERROR, "An object with URI " + uri + " is already in the Document");
            
            // Add the new object to this OwnedObject property
            Identified* parent_obj = (Identified*)this->sbol_owner;
            child_obj->parent = parent_obj;  // Set back-pointer to parent object
            child_obj->identity.set(uri);
            child_obj->persistentIdentity.set(uri);
            
            this->add(*child_obj);
            if (parent_doc)
                child_obj->doc = parent_doc;
            this->validate(child_obj);
            return *child_obj;
        }
    }

//
//    template<>
//    Test& Document::get<Test>(std::string uri)
//    {
//        
//    }
//    template<>
//    Test& OwnedObject<Test>::get(std::string uri)
//    {
//        
//    }
//    
//    template<>
//    Test& OwnedObject<Test>::set(Test& sbol_obj)
//    {
//        
//    }
//    
//    template<>
//    Test& OwnedObject<Test>::add(Test& sbol_obj)
//    {
//        
//    }
//
};