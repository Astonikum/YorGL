package org.yorengine;

@FunctionalInterface
public interface Script extends Component {
    @Override
    void update(SceneObject object, float deltaSeconds);
}
