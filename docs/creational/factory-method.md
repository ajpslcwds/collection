# Factory Method（工厂方法模式）

## 意图
定义一个创建对象的接口，让子类决定实例化哪个类，将对象的创建延迟到子类进行。

## 问题
假设你正在开发一个物流管理系统。系统最初只支持卡车运输（Truck），因此大量业务代码直接耦合在 `Truck` 类上。随着业务扩展，需要新增轮船运输（Ship）和飞机运输（Airplane）。如果直接在业务代码中 `new Truck()`、`new Ship()`，那么每次新增运输方式都要修改所有调用处的代码，违反开闭原则，维护成本急剧上升。你需要一种机制，让创建对象的代码与具体产品类解耦。

## 解决方案
工厂方法模式的核心思想是：用一个虚方法（工厂方法）替代直接调用 `new` 构造具体对象。父类（Creator）定义工厂方法的接口并包含依赖该产品的业务逻辑，子类（ConcreteCreator）覆写工厂方法以返回具体产品。客户端代码只依赖抽象的 Creator 和 Product 接口，完全不知道具体类的存在。当需要新产品时，只需新增一对 ConcreteCreator + ConcreteProduct，无需修改已有代码。

## UML 类图

````mermaid
classDiagram
    class Creator {
        +CreateTransport() unique_ptr~Transport~
        +PlanDelivery()
    }

    class ConcreteCreatorA {
        +CreateTransport() unique_ptr~Transport~
    }

    class ConcreteCreatorB {
        +CreateTransport() unique_ptr~Transport~
    }

    class Product {
        <<interface>>
        +Deliver()
        +GetType() string_view
    }

    class ConcreteProductA {
        +Deliver()
        +GetType() string_view
    }

    class ConcreteProductB {
        +Deliver()
        +GetType() string_view
    }

    Creator <|-- ConcreteCreatorA
    Creator <|-- ConcreteCreatorB
    Product <|.. ConcreteProductA
    Product <|.. ConcreteProductB
    ConcreteCreatorA ..> ConcreteProductA : creates
    ConcreteCreatorB ..> ConcreteProductB : creates
    Creator ..> Product : creates
````

## 适用场景
- 当你在编写创建对象的代码时，无法预见该对象的具体类型，需要由子类来决定
- 当你希望复用已有对象而非每次都创建新实例，将创建逻辑集中到一个位置
- 当你需要为框架或库提供扩展点，让用户能够引入自定义产品类型而不修改框架源码

## 优缺点

**优点**：
- 遵循单一职责原则：产品创建代码集中在 Creator 子类中，业务代码不关心创建细节
- 遵循开闭原则：新增产品只需添加新的 ConcreteCreator 和 ConcreteProduct，无需修改客户端
- 消除了对具体产品类的编译期依赖，客户端只依赖抽象接口
- C++ 中配合 `std::unique_ptr` 自动管理产品对象生命周期，避免内存泄漏

**缺点**：
- 每新增一个产品就需要新增一个对应的 Creator 子类，类的数量会膨胀
- 产品接口（工厂方法返回类型）一旦确定，所有产品都必须实现该接口，灵活性受限

## C++17 实现要点
- **std::unique_ptr** — 工厂方法返回 `std::unique_ptr<Product>` 而非裸指针，自动管理堆对象的生命周期，避免手动 delete
- **std::optional** — `TransportFactory::Create` 返回 `std::optional<std::unique_ptr<Transport>>`，优雅地表达"创建可能失败"的语义，替代 nullptr 检查
- **std::variant** — `CreateTransportVariant` 返回 `std::variant<Truck, Ship, Airplane>`，实现类型安全的产品集合，配合 `std::visit` 进行编译期类型分发
- **std::string_view** — 产品类型标识符使用 `std::string_view`，避免字符串拷贝，零开销抽象
- **if constexpr** — `TransportFactory::Create<Logistics>` 在编译期根据模板参数选择工厂，零运行时开销
- **inline 变量** — 头文件中定义全局常量（如 `kDefaultProduct`），无需 extern 声明，避免 ODR 违反
- **[[nodiscard]]** — 防止忽略工厂方法的返回值，编译期捕捉潜在错误
- **结构化绑定** — 从 `std::optional` 中安全提取值

与传统 C++ 实现（裸 `new`/`delete`、`nullptr` 判空、运行时 `dynamic_cast`）相比，C++17 实现更安全、更表达意图，且编译器能进行更好的优化。

## 相关模式
- **抽象工厂（Abstract Factory）**：工厂方法是抽象工厂的基础。抽象工厂通常包含多个工厂方法，用于创建一整套相关产品；工厂方法只创建单一产品
- **原型（Prototype）**：当产品类的种类很多且不想为每个产品配一个 Creator 子类时，可以让 Creator 通过克隆原型来创建产品，减少子类数量
- **模板方法（Template Method）**：工厂方法是模板方法的特殊形式。模板方法定义算法骨架，工厂方法是其中"创建对象"那一步的具体化

## 代码示例
指向 src/creational/factory-method/main.cpp
