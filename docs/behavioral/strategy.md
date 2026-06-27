# Strategy（策略模式）

## 意图
定义一系列算法，将每个算法封装起来，并使它们可以互相替换，让算法的变化独立于使用算法的客户端。

## 问题
在软件开发中，我们经常会遇到一个问题的多种解决方式。例如：排序可以使用冒泡排序、快速排序、归并排序等；支付可以使用信用卡、支付宝、微信等。如果将这些算法直接硬编码在客户端代码中，会导致：
- 代码中充斥大量 `if-else` 或 `switch-case` 分支
- 添加新算法需要修改已有代码，违反开闭原则
- 算法的实现细节与业务逻辑耦合，难以独立测试和复用

## 解决方案
策略模式的核心思想是**将算法从使用它的类中分离出来**，封装成独立的策略对象。客户端持有一个策略接口的引用，运行时可以动态切换不同的策略实现。这样：
- 每种算法被封装在独立的策略类中，遵循单一职责原则
- 策略可以通过构造函数、setter 方法或工厂注入到上下文中
- 新增算法只需新增一个策略类，无需修改已有代码

## UML 类图

```mermaid
classDiagram
    class Context
    Context : -strategy_: Strategy
    Context : +SetStrategy(Strategy)
    Context : +ExecuteStrategy()

    class Strategy
    <<interface>> Strategy
    Strategy : +Execute()*

    class ConcreteStrategyA
    ConcreteStrategyA : +Execute()

    class ConcreteStrategyB
    ConcreteStrategyB : +Execute()

    class ConcreteStrategyC
    ConcreteStrategyC : +Execute()

    Context o-- Strategy : 持有
    Strategy <|.. ConcreteStrategyA : 实现
    Strategy <|.. ConcreteStrategyB : 实现
    Strategy <|.. ConcreteStrategyC : 实现
```

## 适用场景
- 当一个系统需要在多种算法中动态选择时（如排序算法、压缩算法、加密算法）
- 当一个类有大量条件分支来选择不同行为时，可以将每个分支提取为独立策略
- 当需要隔离算法的实现细节，使客户端不依赖算法的具体实现时

## 优缺点

**优点**：
- 遵循开闭原则：新增策略无需修改已有代码，只需添加新的策略类
- 消除条件分支：用多态替代 `if-else` / `switch-case`，代码更清晰
- 运行时可切换：客户端可以在运行时动态更换策略，提升灵活性

**缺点**：
- 类数量增加：每个策略都是一个类，可能造成类爆炸
- 客户端必须了解所有策略：客户端需要知道有哪些策略可用，才能做出选择
- 简单场景可能过度设计：如果算法只有两三种且不会变化，直接使用条件分支更简单

## C++17 实现要点
- **`std::variant`**：用于类型安全地表示多种策略，替代传统的继承层次结构。通过 `std::visit` 实现编译期多态分发，避免虚函数开销
- **`std::optional`**：用于表示策略可能为空的情况，比裸指针更安全，语义更明确
- **`std::function`**：用于轻量级策略封装，适合简单的、一次性的算法切换场景
- **结构化绑定（Structured Bindings）**：配合 `std::variant` 使用，简化对策略结果的访问
- **`if constexpr`**：在编译期根据策略类型进行条件编译，实现零开销抽象
- **折叠表达式（Fold Expressions）**：用于批量执行多个策略并聚合结果
- **与传统实现的对比**：传统实现依赖虚函数和继承体系，C++17 可以用 `std::variant` + `std::visit` 实现编译期多态，性能更优且类型安全

## 相关模式
- **状态模式（State）**：结构上类似，但意图不同。策略模式由客户端主动选择策略；状态模式由对象内部状态自动切换行为
- **模板方法模式（Template Method）**：模板方法在父类定义算法骨架，子类重写步骤；策略模式将整个算法封装为独立对象
- **工厂模式（Factory）**：常与策略模式配合使用，工厂负责创建具体的策略对象并注入到上下文中

## 代码示例
参见 [src/behavioral/strategy/main.cpp](../../src/behavioral/strategy/main.cpp)
