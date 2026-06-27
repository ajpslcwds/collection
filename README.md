# 设计模式学习笔记

基于 C++17 的 23 个 GoF 设计模式实现与学习笔记。

## 环境要求

- Ubuntu 20.04
- C++17
- CMake 3.14+

## 构建与运行

```bash
mkdir build && cd build
cmake ..
cmake --build .
# 运行示例
./bin/creational/singleton
```

## 设计模式索引

### 创建型模式 (Creational)

| 模式 | 文档 | 代码 | 说明 |
|------|------|------|------|
| Singleton | [文档](docs/creational/singleton.md) | [代码](src/creational/singleton/) | 确保一个类只有一个实例 |
| Factory Method | [文档](docs/creational/factory-method.md) | [代码](src/creational/factory-method/) | 定义创建对象的接口 |
| Abstract Factory | [文档](docs/creational/abstract-factory.md) | [代码](src/creational/abstract-factory/) | 提供创建一系列相关对象的接口 |
| Builder | [文档](docs/creational/builder.md) | [代码](src/creational/builder/) | 将复杂对象的构建与其表示分离 |
| Prototype | [文档](docs/creational/prototype.md) | [代码](src/creational/prototype/) | 通过复制现有对象来创建新对象 |

### 结构型模式 (Structural)

| 模式 | 文档 | 代码 | 说明 |
|------|------|------|------|
| Adapter | [文档](docs/structural/adapter.md) | [代码](src/structural/adapter/) | 将一个类的接口转换成另一个接口 |
| Bridge | [文档](docs/structural/bridge.md) | [代码](src/structural/bridge/) | 将抽象与实现分离 |
| Composite | [文档](docs/structural/composite.md) | [代码](src/structural/composite/) | 将对象组合成树形结构 |
| Decorator | [文档](docs/structural/decorator.md) | [代码](src/structural/decorator/) | 动态地给对象添加额外职责 |
| Facade | [文档](docs/structural/facade.md) | [代码](src/structural/facade/) | 为子系统提供统一的高层接口 |
| Flyweight | [文档](docs/structural/flyweight.md) | [代码](src/structural/flyweight/) | 共享细粒度对象，减少内存使用 |
| Proxy | [文档](docs/structural/proxy.md) | [代码](src/structural/proxy/) | 为另一个对象提供代理以控制访问 |

### 行为型模式 (Behavioral)

| 模式 | 文档 | 代码 | 说明 |
|------|------|------|------|
| Chain of Responsibility | [文档](docs/behavioral/chain-of-responsibility.md) | [代码](src/behavioral/chain-of-responsibility/) | 将请求沿着处理链传递 |
| Command | [文档](docs/behavioral/command.md) | [代码](src/behavioral/command/) | 将请求封装为对象 |
| Iterator | [文档](docs/behavioral/iterator.md) | [代码](src/behavioral/iterator/) | 提供顺序访问聚合对象元素的方法 |
| Mediator | [文档](docs/behavioral/mediator.md) | [代码](src/behavioral/mediator/) | 用中介对象封装一组对象的交互 |
| Memento | [文档](docs/behavioral/memento.md) | [代码](src/behavioral/memento/) | 在不破坏封装的前提下捕获和恢复状态 |
| Observer | [文档](docs/behavioral/observer.md) | [代码](src/behavioral/observer/) | 定义一对多依赖关系 |
| State | [文档](docs/behavioral/state.md) | [代码](src/behavioral/state/) | 允许对象在内部状态改变时改变其行为 |
| Strategy | [文档](docs/behavioral/strategy.md) | [代码](src/behavioral/strategy/) | 定义一系列算法，将每个算法封装起来 |
| Template Method | [文档](docs/behavioral/template-method.md) | [代码](src/behavioral/template-method/) | 定义算法骨架，将某些步骤延迟到子类 |
| Visitor | [文档](docs/behavioral/visitor.md) | [代码](src/behavioral/visitor/) | 表示作用于对象结构中各元素的操作 |
| Interpreter | [文档](docs/behavioral/interpreter.md) | [代码](src/behavioral/interpreter/) | 定义语言的文法，并建立解释器 |

## 学习路径建议

1. **入门**：Singleton → Factory Method → Observer → Strategy
2. **进阶**：Abstract Factory → Decorator → Command → State
3. **高级**：Builder → Composite → Visitor → Interpreter
