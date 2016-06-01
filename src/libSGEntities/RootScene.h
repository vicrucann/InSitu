#ifndef ROOTSCENE
#define ROOTSCENE

/* RootScene
 * It contains all the entities that will appear on the scene.
 * It includes both user scene and utility entities (axis).
 * The user scene has the following structure (tree branch example):
 * [Root] -> <Transform1> -> (Canvas11) -> `Stroke111`
 *        -> <Transform2> -> (Canvas21)
 *                        -> (Canvas22) -> `Stroke221`
 *                                      -> `Stroke222`
 * ...
 *       -> {Switch} -> {Axes}
 *
 * Where we denote:
 * [] - an osg::Group inherited
 * <> - osg::MatrixTransform inherited
 * () - osg::Geode inherited
 * `` - osg::Drawable inherited
 * {} - other osg inherited types such as camera or switch nodes
 *
 * When performing addCanvas() or deleteCanvas(), this RootScene
 * tries to delete the canvases directly from m_userScene through the
 * API methods.
 */

#include <iostream>
#include <string>
#include <string>

#include <osg/ref_ptr>
#include <osg/AutoTransform>

#include "Settings.h"
#include "../libGUI/hudcamera.h"
#include "UserScene.h"
#include "Entity2D.h"
#include "Stroke.h"
#include "Photo.h"
#include "Bookmarks.h"
#include "../libGUI/ListWidget.h"
#include "ToolGlobal.h"
#include "SceneState.h"

#include <QUndoStack>
#include <QModelIndex>

class AddStrokeCommand;
namespace entity{
class SceneState;
class UserScene;
class Bookmarks;
}

/*! \class RootScene
 * Class description
*/
class RootScene : public osg::Group {
public:
    RootScene(QUndoStack* undoStack);

    entity::UserScene* getUserScene() const;
    void setFilePath(const std::string& name);
    bool isSetFilePath() const;
    bool isSavedToFile() const;
    bool isEmptyScene() const;
    void clearUserData();

    void setToolsVisibility(bool vis);
    bool getToolsVisibility() const;
    void setAxesVisibility(bool vis);
    bool getAxesVisibility() const;

    bool writeScenetoFile();
    bool exportSceneToFile(const std::string& name);
    bool loadSceneFromFile();

    int getStrokeLevel() const;
    int getCanvasLevel() const;
    int getPhotoLevel() const;

    void addCanvas(const osg::Matrix& R, const osg::Matrix& T);
    void addCanvas(const osg::Vec3f& normal, const osg::Vec3f& center);
    void addCanvas(const osg::Matrix& R, const osg::Matrix& T, const std::string& name);
    void addStroke(float u, float v, cher::EVENT event);
    void selectAllStrokes();
    void addPhoto(const std::string& fname);
    void addBookmark(BookmarkWidget* widget, const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up, const double& fov);
    void addBookmarkTool(const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up);
    void updateBookmark(BookmarkWidget* widget, int row);
    void deleteBookmark(BookmarkWidget* widget, const QModelIndex& index);
    void deleteBookmarkTool(int first, int last);
    void resetBookmarks(BookmarkWidget* widget);

    /*! A method to set visibility of all the bookmarks tools. The visibility cannot be set for individual bookmarks tool, only for the
     * whole group.
     * \param vis is the visibility flag: true for visibile and false for invisible state. */
    void setBookmarkToolVisibility(bool vis);

    /*! \return A visibility flag for the set of all the bookmark tools. Since the visibility cannot be set for individual bookmarks,
     * the returned value represents the visibility of the whole group: true for visibile and false for being invisible. */
    bool getBookmarkToolVisibility() const;

    void eraseStroke(entity::Stroke* stroke, int first, int last, cher::EVENT event);

    bool setCanvasCurrent(entity::Canvas* cnv);
    bool setCanvasPrevious(entity::Canvas* cnv);
    void setCanvasesButCurrent(bool enable);
    entity::Canvas* getCanvasCurrent() const;
    entity::Canvas* getCanvasPrevious() const;
    entity::Bookmarks* getBookmarksModel() const;

    void editCanvasOffset(const osg::Vec3f& translate, cher::EVENT event);
    void editCanvasRotate(const osg::Quat& rotation, const osg::Vec3f& center3d, cher::EVENT event);
    void editCanvasClone(const osg::Vec3f& translate, cher::EVENT event);
    void editCanvasSeparate(const osg::Vec3f& translate, cher::EVENT event);
    void editCanvasDelete(entity::Canvas* canvas);

    void editPhotoDelete(entity::Photo* photo, entity::Canvas* canvas);
    void editPhotoPush(entity::Photo* photo, entity::Canvas* source, entity::Canvas* destination);

    void editStrokesPush(osg::Camera *camera);
    void editStrokesMove(double u, double v, cher::EVENT event);
    void editStrokesScale(double u, double v, cher::EVENT event);
    void editStrokesRotate(double u, double v, cher::EVENT event);
    void editStrokeDelete(entity::Stroke* stroke);

    void copyToBuffer();
    void cutToBuffer();
    void pasteFromBuffer();

    /*! \return A pointer on SceneState. Note: this pointer is not managed by smart pointers and must
     * be deleted manually.
     * \sa entity::SceneState::stripDataFrom() */
    entity::SceneState* getSceneState() const;

    /*! \param A pointer on SceneState instance which is served as a source for RootScene settings.
     * \return Whether the operation was performed successfully (no size discrepancies found) or not. */
    bool setSceneState(const entity::SceneState* state);

protected:
    ~RootScene();
    void printScene();

private:
    osg::ref_ptr<entity::UserScene> m_userScene;
    entity::AxisGlobalTool* m_axisTool;
    osg::Group* m_bookmarkTools;
    std::vector< osg::ref_ptr<entity::Entity2D> > m_buffer; /* copy-paste buffer */
    QUndoStack* m_undoStack;
    bool m_saved;
};

#endif // SCENE

