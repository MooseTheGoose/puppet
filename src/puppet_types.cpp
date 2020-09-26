#include "puppet_types.hpp"

const string PUPPET_LINE_SEP = "\n";

string PuppetBigInt::to_string() {
  return std::to_string(this->num);
}

string PuppetFloat::to_string() {
  return std::to_string(this->num);
}

string PuppetString::to_string() {
  return "\"" + this->str + "\""; 
}

string PuppetList::to_string() {
  string buf = "[";

  for(PuppetData pd : this->data) {
    buf += (pd.to_string() + ",");
  }
  if(buf.back() == ',') {
    buf.pop_back();
  }
  buf += "]";

  return buf;
}

string PuppetObject::to_string() {
  string buf = "{";

  for(int i = 0; i < this->keys.size(); i++) {
    buf += (this->keys[i].to_string() + ":" + this->values[i].to_string() + ",");
  }
  if(buf.back() == ',') {
    buf.pop_back();
  }
  buf += "}";
 
  return buf;
}

string PuppetData::to_string() {
  string rep = "";

  switch(identifier) {
    case TYPE_BIGINT:
      rep = this->i->to_string();
      break;
    case TYPE_FLOAT:
      rep = this->f->to_string();
      break;
    case TYPE_STRING:
      rep = this->s->to_string();
      break;
    case TYPE_LIST:
      rep = this->arr->to_string();
      break; 
    case TYPE_OBJECT:
      rep = this->obj->to_string();
      break; 
  }

  return rep;
}