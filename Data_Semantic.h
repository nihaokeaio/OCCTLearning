//
// Created by gmh on 24-8-27.
//

#ifndef ROSEFINCH_DATA_SEMANTIC_H
#define ROSEFINCH_DATA_SEMANTIC_H



#define DECLARE_SEMANTIC_SUCCESSOR_PARAM_CONSTRUCTOR(Successor) \
  public:                                                       \
  Successor(const Data_Semantic<Successor> &semantic) { \
    for (auto &s : semantic.GetSemanticItoms()){ \
      AddSemantic(s); \
    } \
  } \
  Successor(Data_Semantic<Successor> &&semantic) { \
    for (auto &s : semantic.GetSemanticItoms()){ \
      AddSemantic(s); \
    } \
  } \
  explicit Successor(std::string_view semantic) : Data_Semantic<Successor>(semantic){} \
  explicit Successor(const std::vector<std::string> &semantics) : Data_Semantic<Successor>(semantics){}; \
 private: \

/*
 * 实现子类需要遵循以下规范：
 *
class Your_Data_Semantic: public Data_Semantic<Your_Data_Semantic>{
 public:
  // 添加更多的语义
  inline static const std::string Unknown = "Unknown";

  // 该列表需要和上面的语义一一对应
  inline static const std::vector<std::string_view> Views = {
      std::string_view(Unknown)
  };

 public:
  Your_Data_Semantic() {
    // init or not
  };
  DECLARE_SEMANTIC_SUCCESSOR_PARAM_CONSTRUCTOR(Your_Data_Semantic)
};

 * */

template<class Successor>
class Data_Semantic {
 public:
  Data_Semantic() = default;
  explicit Data_Semantic(std::string_view semantic) : m_Semantic({semantic}) {
    CheckValid(semantic);
  }
  explicit Data_Semantic(const std::vector<std::string> & semantics) {
    for(auto &s : semantics){
      if(Contain(s)){
        continue;
      }
      m_Semantic.push_back(PreDefined(s));
    }
  };

  inline static const std::string delimiter = "::";

  void CheckValid(const std::string_view &semantic) {
    auto successor_p = static_cast<Successor *>(this);
    bool found =
        std::any_of(successor_p->Views.begin(), successor_p->Views.end(), [&semantic](std::string_view view){
          return semantic==view;
        });
    if(!found){
      throw std::runtime_error("POI Semantic Invalid! Plz get string view from POI_Data_Semantic const static field!");
    }
  }

  std::string_view PreDefined(const std::string & query){
    auto successor_p = static_cast<Successor *>(this);
    for(auto view : successor_p->Views){
      if(view == query){
        return {view};
      }
    }
    throw std::runtime_error("POI Semantic did not predefined!");
  }

  bool AddSemantic(const std::string_view &semantic) {
    CheckValid(semantic);
    if(Contain(semantic)){
      return false;
    }
    m_Semantic.push_back(semantic);
    return true;
  };

  void RemoveSemantic(const std::string_view &semantic) {
    m_Semantic.erase(std::remove_if(m_Semantic.begin(), m_Semantic.end(),
                                    [&semantic](std::string_view s) {
                                      return s == semantic;
                                    }),
                    m_Semantic.end());
  };

  void ClearSemantic(){
    m_Semantic.clear();
  }

  bool Contain(const std::string_view &semantic) const {
    return std::any_of(m_Semantic.begin(), m_Semantic.end(),
                       [&semantic](std::string_view s) {
                         return s == semantic;
                       });
  };

  const std::vector<std::string_view> & GetSemanticItoms() const {
    return m_Semantic;
  };

  bool operator==(const Data_Semantic &rhs) const {
    auto semantic_itoms = GetSemanticItoms();
    bool result = std::all_of(semantic_itoms.begin(), semantic_itoms.end(),
                              [&rhs](std::string_view semantic) {
                                return rhs.Contain(semantic);
                              });
    return result;
  };

  [[nodiscard]]
  std::string ToString() const {
    std::string result;
    for (size_t i=0 ; i < m_Semantic.size(); i++) {
      result += m_Semantic[i];
      if (i != m_Semantic.size() - 1) {
        result += delimiter;
      }
    }
    return result;
  }

 protected:
  std::vector<std::string_view> m_Semantic;
};


#endif//ROSEFINCH_DATA_SEMANTIC_H
