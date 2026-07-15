#  请在此处编写代码，实现使用 KNN 算法分类
# ====================Begin===================
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder, StandardScaler
from sklearn.neighbors import KNeighborsClassifier
from sklearn.metrics import accuracy_score, precision_score, recall_score, f1_score

def run_knn_person_classification(train_size,random_state):
    # 1. 数据加载
    df = pd.read_csv("500_Person_Gender_Height_Weight_Index.csv")
    # 2. 自变量因变量
    X = df[["Gender", "Height", "Weight"]]
    y = df["Index"]
    # 3. 性别编码
    le = LabelEncoder()
    X["Gender"] = le.fit_transform(X["Gender"])
    # 4. 划分数据集
    X_train, X_test, y_train, y_test = train_test_split(
        X, y,
        train_size=train_size,
        random_state=random_state,
        stratify=y
    )
    # 5. 标准化
    scaler = StandardScaler()
    X_train_scaled = scaler.fit_transform(X_train)
    X_test_scaled = scaler.transform(X_test)
    # 6. KNN训练
    model = KNeighborsClassifier(n_neighbors=5, weights="distance", metric="euclidean")
    model.fit(X_train_scaled, y_train)
    # 7. 预测评估
    y_pred = model.predict(X_test_scaled)
    accuracy = accuracy_score(y_test, y_pred)
    precision = precision_score(y_test, y_pred, average="weighted")
    recall = recall_score(y_test, y_pred, average="weighted")
    f1 = f1_score(y_test, y_pred, average="weighted")
    return float(accuracy), float(precision), float(recall), float(f1)
# ====================End===================
