# 用列表嵌套字典构建电影库
movie_database = [
    {"title": "流浪地球2", "type": "科幻", "rating": 8.3, "year": 2023},
    {"title": "你好，李焕英", "type": "喜剧", "rating": 8.1, "year": 2021},
    {"title": "满江红", "type": "悬疑", "rating": 7.6, "year": 2023},
    {"title": "人生大事", "type": "剧情", "rating": 7.3, "year": 2022},
    {"title": "疯狂的石头", "type": "喜剧", "rating": 8.6, "year": 2006},
    {"title": "星际穿越", "type": "科幻", "rating": 9.4, "year": 2014},
    {"title": "唐人街探案3", "type": "喜剧", "rating": 5.3, "year": 2021}
]

# 获取用户偏好:用户的电影类型偏好和最低评分要求
def get_user_preference():
    # 输入喜欢的电影类型（如：喜剧/科幻/悬疑）
    user_type = input()
    #数据容器 + 类型转换（处理用户输入的字符串为浮点数）
    while True:  # 循环：确保用户输入合法的评分
        try:
            # 输入可接受的最低评分（如：8.0）
            min_rating = float(input())
            if 0 <= min_rating <= 10:
                break
            else:
                print("评分需在0-10之间,请重新输入!")
        except ValueError:
            print("请输入数字（如8.0）,不要输入文字!")
    return user_type, min_rating

# 根据用户偏好筛选电影
def recommend_movies(user_type, min_rating, movie_db):
    """
    :param user_type: 用户喜欢的类型
    :param min_rating: 最低评分要求
    :param movie_db: 电影库数据
    :return: 符合条件的电影列表
    """
    # 请在此处编写代码
    # ====================Begin====================
    recommended = []
    for movie in movie_db:
        if movie["type"] == user_type and movie["rating"] >= min_rating:
            recommended.append(movie)


    # ====================end=====================
    return recommended

# 输出推荐结果
def show_recommendation(recommended_movies):
    """展示推荐结果"""
    print("===== 为你推荐的电影 ======")

    # 请在此处编写代码
    # ====================Begin====================
    if not recommended_movies:  # 条件判断：无推荐的情况
        print("暂无符合你偏好的电影！")
    else:
        for idx, movie in enumerate(recommended_movies, 1):
            print(f"{idx}. 标题：{movie['title']} | 类型：{movie['type']} | 评分：{movie['rating']} | 年份：{movie['year']}")

    # ====================end=====================

# 主程序入口
if __name__ == "__main__":

    # 请在此处编写代码
    # ====================Begin====================
    prefer_type, min_score = get_user_preference()
    result_list = recommend_movies(prefer_type, min_score, movie_database)
    show_recommendation(result_list)
    # ====================end=====================
