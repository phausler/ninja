// Copyright 2011 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "manifest_writer.h"
#include "state.h"
#include "graph.h"

using namespace ninja;

ManifestWriter::ManifestWriter(State* state, FileWriter* file_writer) :
  content_(""),
  state_(state),
  file_writer_(file_writer) {
  
}

bool ManifestWriter::WriteBindings(BindingEnv& env) {
  for (map<string, string>::iterator it = env.bindings_.begin();
     it != env.bindings_.end();
     ++it) {
    content_ += it->first + " = " + it->second + "\n\n";
  }
  return true;
}

bool ManifestWriter::WritePool(string name, Pool* pool, string* err) {
  content_ += "pool " + pool->name() + "\n";
  content_ += "\tdepth = " + to_string(pool->depth()) + "\n\n";
  return true;
}

bool ManifestWriter::WriteRule(string name, const Rule* rule, string* err) {
  content_ += "rule " + rule->name() + "\n";
  for (map<string, EvalString>::iterator it = ((Rule *)rule)->bindings_.begin();
     it != ((Rule *)rule)->bindings_.end();
     ++it) {
    if ((it->first == "rspfile" ||
       it->first == "rspfile_content") &&
      it->second.parsed_.empty()) {
      continue;
    }

    content_ += "\t" + it->first + " ";
    
    for (EvalString::TokenList::iterator e = it->second.parsed_.begin();
       e != it->second.parsed_.end();
       ++e) {
      if (e->second == EvalString::RAW) {
        content_ += e->first;
      } else {
        content_ += "$" + e->first;
      }
    }
    content_ += "\n";
  }
  content_ += "\n";
  return true;
}

bool ManifestWriter::WriteEdge(Edge* edge, string* err) {
  content_ += "build ";
  for (vector<Node*>::iterator it = edge->outputs_.begin();
     it != edge->outputs_.end();
     ++it) {
    content_ += (*it)->path();
    if (it + 1 != edge->outputs_.end()) {
      content_ += ", ";
    }
  }
  
  content_ += ": " + edge->rule().name() + " ";
  for (vector<Node*>::iterator it = edge->inputs_.begin();
     it != edge->inputs_.end();
     ++it) {
    content_ += (*it)->path();
    if (it + 1 != edge->inputs_.end()) {
      content_ += ", ";
    }
  }
  
  content_ += "\n";
  return true;
}

bool ManifestWriter::Write(const string& filename, string* err) {
  
  if (!WriteBindings(state_->bindings_)) {
    return false;
  }
  
  for (map<string, Pool*>::iterator it = state_->pools_.begin();
     it != state_->pools_.end();
     ++it) {
    
    if (!WritePool(it->first, it->second, err)) {
      return false;
    }
  }
  
  for (map<string, const Rule*>::iterator it = state_->rules_.begin();
     it != state_->rules_.end();
     ++it) {
    if (!WriteRule(it->first, it->second, err)) {
      return false;
    }
  }
  
  for (vector<Edge*>::iterator it = state_->edges_.begin();
     it != state_->edges_.end();
     ++it) {
    if (!WriteEdge(*it, err)) {
      return false;
    }
  }
  
  return file_writer_->WriteFile(filename, content_, err);
}